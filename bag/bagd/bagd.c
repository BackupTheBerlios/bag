/***************************************************************************
                          bagd.c  -  bagd main loop
                             -------------------
    begin                : Mon Sep 30 2002
    copyright            : (C) 2002 by Konrad Rosenbaum
    email                : konrad.rosenbaum@gmx.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../config.h"
#include "sql.h"
#include <log.h>
#include "query.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define _GNU_SOURCE
#include <getopt.h>

#include "bagstream.h"
#include "bagssl.h"

#include "bagd.h"
#include "../libbag/debug.h"

/*database configuration*/
char *connectstring=0;
char *servername="";

/*is this virtualhost/managed mode?*/
int vhost=0;

/* nsockets = number of listening sockets
 * sockets = array of socket fd's
 * isssl = bool array: is the corresponding socket listening for SSL connections
 * socketpaths = path to all AF_LOCAL sockets
 *
 * nchildren = number of currently active children
 * children = PID of all running child processes
 */
static int nsockets=0,*sockets=0,*isssl=0,nchildren=0;
static char **socketpaths=0;
static pid_t *children;



/*close all sockets, does not delete paths, since also called when a child is spawned*/
static void closeall()
{
        int i;
        for(i=0;i<nsockets;i++)
                close(sockets[i]);
}

/*remove complete configuration*/
static void deconfig()
{
        int i;
        for(i=0;i<nsockets;i++){
                close(sockets[i]);
                if(socketpaths[i]){
                        unlink(socketpaths[i]);
                        free(socketpaths[i]);
                }
        }
        if(sockets)free(sockets);
        if(isssl)free(isssl);
        if(socketpaths)free(socketpaths);
        sockets=0;
        isssl=0;
        socketpaths=0;
}

/*load configuration from the server, only sockets are supported,
 the remainder is loaded inide bagchild*/
static void config()
{
        dbConn *con;
        int i;
        char*s;
        dbResult *res;
        con=dbConnect(connectstring);
        if(con==0){
                exit(1);
        }

        res=query(con,SQL_GETOPTIONS,servername);
        if(!res){
                log(LOG_ERR,"Unable to get configuration.");
                dbClose(con);
                exit(1);
        }

        nsockets=dbNumRows(res);

        sockets=malloc(sizeof(int)*nsockets);
        isssl=malloc(sizeof(int)*nsockets);
        socketpaths=malloc(sizeof(char*)*nsockets);

        for(i=0;i<nsockets;i++){
                char*x;
                int t;/*0=error, 1=TCP, 2=AF_UNIX, 3=TCPv6 ...*/
                s=dbGetString(res,i,1);
                sockets[i]=-1;t=0;
                socketpaths[i]=0;
                if(!strncmp("tcp:",s,4)){t=1;x=s+4;isssl[i]=0;}else
                if(!strncmp("tcp6:",s,5)){t=3;x=s+5;isssl[i]=0;}else
                if(!strncmp("local:",s,6)){t=2;x=s+6;isssl[i]=0;}else
                if(!strncmp("unix:",s,5)){t=2;x=s+5;isssl[i]=0;}
#ifdef HAVE_GNUTLS
                else
                if(!strncmp("ssl:",s,4)){t=1;x=s+4;isssl[i]=1;}else
                if(!strncmp("ssl6:",s,5)){t=3;x=s+5;isssl[i]=1;}else
                if(!strncmp("slocal:",s,7)){t=2;x=s+7;isssl[i]=1;}else
                if(!strncmp("sunix:",s,6)){t=2;x=s+6;isssl[i]=1;}
#endif
                else{
                        log(LOG_WARNING,"Failed opening socket %s, unknown type.",s);
                        continue;
                }
                switch(t){
                        case 1:{/*tcp*/
                                struct sockaddr_in si;
                                int sopt,port;
                                sockets[i]=socket(PF_INET,SOCK_STREAM,0);
                                if(sockets[i]==-1){
                                        log(LOG_WARNING,"Unable to allocate TCP socket %s: %s",s,strerror(errno));
                                        continue;
                                }
                                sopt=1;
                                if(setsockopt(sockets[i],SOL_SOCKET,SO_REUSEADDR,&sopt,sizeof(sopt))==-1)
                                        log(LOG_WARNING,"Unable to set reuse socket option on %s.",s);
                                si.sin_family=AF_INET;
                                si.sin_addr.s_addr=htonl(INADDR_ANY);
                                si.sin_port=port=htons(atoi(x));
                                if(bind(sockets[i],&si,sizeof(si))==-1){
                                        log(LOG_WARNING,"Unable to bind TCP socket %s: %s",s,strerror(errno));
                                        close(sockets[i]);
                                        sockets[i]=-1;
                                        continue;
                                }
                                if(listen(sockets[i],5)==-1){
                                        log(LOG_WARNING,"Unable to set TCP socket %s to listen mode: %s",s,strerror(errno));
                                        close(sockets[i]);
                                        sockets[i]=-1;
                                        continue;
                                }
                                log(LOG_INFO,"Successfully initialized TCP socket %s. Port=%i FD=%i.",s,ntohs(port),sockets[i]);
                                break;
                        }
                        case 3:{/*tcp6*/
                                struct sockaddr_in6 si;
                                int sopt,port;
                                sockets[i]=socket(PF_INET6,SOCK_STREAM,0);
                                if(sockets[i]==-1){
                                        log(LOG_WARNING,"Unable to allocate TCPv6 socket %s: %s",s,strerror(errno));
                                        continue;
                                }
                                if(setsockopt(sockets[i],SOL_SOCKET,SO_REUSEADDR,&sopt,sizeof(sopt))==-1)
                                        log(LOG_WARNING,"Unable to set reuse socket option on %s.",s);
                                si.sin6_family=AF_INET6;
                                si.sin6_addr=in6addr_any;
                                si.sin6_port=port=htons(atoi(x));
                                if(bind(sockets[i],&si,sizeof(si))==-1){
                                        log(LOG_WARNING,"Unable to bind TCPv6 socket %s: %s",s,strerror(errno));
                                        close(sockets[i]);
                                        sockets[i]=-1;
                                        continue;
                                }
                                if(listen(sockets[i],5)==-1){
                                        log(LOG_WARNING,"Unable to set TCPv6 socket %s to listen mode: %s",s,strerror(errno));
                                        close(sockets[i]);
                                        sockets[i]=-1;
                                        continue;
                                }
                                log(LOG_INFO,"Successfully initialized TCPv6 socket %s. Port=%i FD=%i.",s,ntohs(port),sockets[i]);
                                break;
                        }
                        case 2:{/*local*/
                                struct sockaddr_un su;
                                sockets[i]=socket(PF_UNIX,SOCK_STREAM,0);
                                if(sockets[i]==-1){
                                        log(LOG_WARNING,"Unable to allocate AF_UNIX socket %s: %s",s,strerror(errno));
                                        continue;
                                }
                                su.sun_family=AF_UNIX;
                                strcpy(su.sun_path,x);
                                unlink(x);
                                if(bind(sockets[i],&su,sizeof(su))==-1){
                                        log(LOG_WARNING,"Unable to bind AF_UNIX socket %s: %s",s,strerror(errno));
                                        close(sockets[i]);
                                        sockets[i]=-1;
                                        continue;
                                }
                                if(listen(sockets[i],5)==-1){
                                        log(LOG_WARNING,"Unable to set AF_UNIX socket %s to listen mode: %s",s,strerror(errno));
                                        close(sockets[i]);
                                        sockets[i]=-1;
                                        unlink(x);
                                        continue;
                                }
                                log(LOG_INFO,"Successfully initialized socket %s.",s);
                                socketpaths[i]=malloc(strlen(x)+1);
                                strcpy(socketpaths[i],x);
                                break;
                        }
                        default:/*error/unknown*/
                                log(LOG_WARNING,"Unknown socket type or internal error: %s.",s);
                                continue;
                                break;
                }
        }
        
        dbFree(res);
        dbClose(con);
}

/*add a child process to the list*/
void addchild(pid_t pid)
{
        int i;
        if(children==0){
                children=malloc(sizeof(pid_t)*16);
                nchildren=16;
                for(i=0;i<16;i++)children[i]=0;
        }
        for(i=0;i<nchildren;i++){
                if(children[i]==0){
                        children[i]=pid;
                        return;
                }
        }
        nchildren+=16;
        children=realloc(children,sizeof(pid_t)*nchildren);
        children[nchildren-16]=pid;
        for(i=nchildren-15;i<nchildren;i++)
                children[i]=0;
}

/*delete a child process from the list*/
void delchild(pid_t pid)
{
        int i;
        for(i=0;i<nchildren;i++)
                if(pid==children[i])
                        children[i]=0;
}

/*standard signal handler*/
static void bagdsighandler(int sig)
{
        switch(sig){
                case SIGHUP:{
                        int i;
                        /*reread config*/
                        deconfig();
                        config();
                        /*propagate signal*/
                        for(i=0;i<nchildren;i++)
                                if(children[i]>0)
                                        kill(children[i],SIGHUP);
                        break;
                }
                case SIGCHLD:{
                        int stat;
                        char*type="?unknown event?";
                        pid_t pid;
                        pid=waitpid(-1,&stat,WNOHANG|WUNTRACED);
                        if(pid>0 && (WIFEXITED(stat) || WIFSIGNALED(stat)))
                                delchild(pid);

                        if(WIFEXITED(stat))type="exited normally";
                        if(WIFSIGNALED(stat))type="killed by signal";
                        log(LOG_DEBUG,"SIGCHLD: pid=%i status=%i signal=%i %s",(int)pid,
                                WEXITSTATUS(stat),WTERMSIG(stat),type);
                        break;
                }
                default:
                        deconfig();
                        log(LOG_INFO,"got signal %i, exiting",sig);
                        exit(0);
                        break;
        }
}

/*create a child process to handle a connection*/
static void spawnchild(int fd,int usessl)
{
        pid_t pid;
        pid=fork();
        if(pid<0){
                log(LOG_WARNING,"Unable to spawn another process: %s.",strerror(errno));
                close(fd);
                return;
        }else
        if(pid==0){/*child*/
                struct s_sockethandler*sh;
                /*create handler*/
                if(usessl)sh=newsslhandler(fd);
                else sh=newstreamhandler(fd);
                /*close all server sockets (to enable clients to run on,
                  while the server reconfigures)*/
                closeall();
                /*switch to child mode*/
                bagchild(sh,connectstring);
                exit(0);
        }else{/*parent*/
                addchild(pid);
                close(fd);
                return;
        }
}

/*go to daemon mode*/
void daemonize()
{
        close(0);close(1);close(2);
        if(fork())exit(0);
        tosyslog=1;
        chdir("/");
}

/*output help text*/
#define HELP "\
\tOptions:\n\
\t\t-f --file <filename>\n\
\t\t\tread connection string from file\n\n\
\t\t-h --help\n\
\t\t\toutput this help text and exit\n\n\
\t\t-v --version\n\
\t\t\toutput version number and exit\n\n\
\t\t-d --debug\n\
\t\t\tdon't go into daemon mode\n\n\
\t\t-q --querydebug\n\
\t\t\tdebug queries\n\n\
\t\t-n --name --servername <name>\n\
\t\t\tset the servername, used to distinguish configuration\n\
\t\t\tof different servers on the same database\n\n\
\t\t-m --managed\n\
\t\t\tswitch to managed mode, connection socket is expected at stdin\n\n\
\tConnection Parameters:\n\
\t\tdbname=name of the database\n\
\t\thostaddr=host to connect to (default: localhost)\n\
\t\tport=port to connect to (default: 5432)\n\
\t\tuser=DB-user to use for connection\n\
\t\tpassword=DB-Password\n"
static void help(const char*name)
{
        fprintf(stderr, PACKAGE " version " VERSION "\nUsage: %s [options] [DB-connection string]\n" HELP,name);
}

/*parse commandline*/
static int godaemon=1;
static char sopt[]="hvf:dn:qm";
static struct option lopt[]={
        {"help",0,0,'h'},
        {"version",0,0,'v'},
        {"file",1,0,'f'},
        {"debug",0,0,'d'},
        {"querydebug",0,0,'q'},
        {"servername",1,0,'n'},
        {"name",1,0,'n'},
        {"managed",0,0,'m'},
        {0,0,0,0}
};
static void handleoptions(int argc,char**argv)
{
        int c,l=0,fd;
        char*argv0=argv[0],*file=0;
        while(1){
                c=getopt_long(argc,argv,sopt,lopt,0);
                if(c<0)break;

                switch(c){
                        case 'v':
                        case 'h':help(argv0);exit(0);break;

                        case 'd':godaemon=0;break;

                        case 'q':querydebug++;break;
                        
                        case 'f':file=optarg;break;

                        case 'n':servername=optarg;break;

                        case 'm':vhost=1;break;

                        default:help(argv0);exit(1);break;
                }
        }
        c=optind;
        while(c<argc)l+=strlen(argv[c++])+1;
        if(file){
                fd=open(file,O_RDONLY);
                if(fd<0){
                        log(LOG_ERR,"Error opening file %s: %s",file,strerror(errno));
                        exit(1);
                }
                c=lseek(fd,0,SEEK_END);
                if(c>=0){
                        lseek(fd,0,SEEK_SET);
                }else{
                        log(LOG_ERR,"Error trying to get file length %s: %s",file,strerror(errno));
                        exit(1);
                }
        }
        connectstring=malloc(l);
        if(connectstring==0){
                log(LOG_ERR,"Cannot allocate enough memory for DB connection string.");
                exit(1);
        }
        if(file){
                read(fd,connectstring,c);
                connectstring[c]=0;
        }else *connectstring=0;
        while(optind<argc){
                if(*connectstring)strcat(connectstring," ");
                strcat(connectstring,argv[optind++]);
        }
}

int main(int argc,char**argv)
{
        int i,l;
        struct sigaction sa;
        if(argc<2){
                help(argv[0]);
                exit(1);
        }

        handleoptions(argc,argv);
        if(connectstring==0){
                help(argv[0]);
                exit(1);
        }
        
        /*init signals*/
        sa.sa_sigaction=0;
        sa.sa_handler=bagdsighandler;
        sa.sa_flags=SA_NOMASK|SA_RESTART;
        sigaction(SIGHUP,&sa,0);
        sigaction(SIGTERM,&sa,0);
        sigaction(SIGQUIT,&sa,0);
        sigaction(SIGCHLD,&sa,0);
        sigaction(SIGPIPE,&sa,0);
        sigaction(SIGINT,&sa,0);

        /*if we are in managed mode, we immediately switch to childmode*/
        if(vhost){
                struct s_sockethandler*sh;
                /*redirect output...*/
                tosyslog=1;
                chdir("/");
                /*create handler*/
                sh=newstreamhandler(0);
                /*switch to child mode*/
                bagchild(sh,connectstring);
                /*leave*/
                exit(0);
        }/*else go to normal server mode*/

        /*check config*/
        config();

        /*finally daemonize*/
        if(godaemon)daemonize();
        
        /*main loop*/
        for(;;){
                fd_set rd;
                int m;
                FD_ZERO(&rd);
                l=0;m=0;
                for(i=0;i<nsockets;i++)
                        if(sockets[i]>=0){
                                l++;
                                FD_SET(sockets[i],&rd);
                                if(sockets[i]>m)m=sockets[i];
                        }
                if(l==0){
                        log(LOG_ERR,"Fatal error: no listening sockets remaining, exiting.");
                        exit(1);
                }
                i=select(m+1,&rd,0,0,0);
                if(i==-1){
                        int e=errno;
                        if(e==EINTR)continue;
                        log(LOG_ERR,"Fatal error on select: %s, exiting.",strerror(e));
                        exit(1);
                }
                for(i=0;i<nsockets;i++)
                        if(sockets[i]>=0 && FD_ISSET(sockets[i],&rd)){
                                int fd=accept(sockets[i],0,0);
                                if(fd<0){
                                        int e=errno;
                                        if(e==EAGAIN||e==EWOULDBLOCK)continue;
                                        log(LOG_ERR,"Internal error on accept(%i,0,0): %s, exiting.",fd,strerror(e));
                                        exit(1);
                                }
                                spawnchild(fd,isssl[i]);
                        }

        }

        /*shouldn't be reached, but anyway...*/
        return 0;
}
