/***************************************************************************
                          vhost.c  -  virtual host stub
                             -------------------
    begin                : Sun Sep 21 2003
    copyright            : (C) 2003 by Konrad Rosenbaum
    email                : konrad@silmor.de
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

#include "../libbag/debug.h"


char *vhostfile="/etc/bagd.vhosts";
int port=2410,ipv6=0;
char *bagd="bagd";
int sock=-1;

/*output help text*/
#define HELP "\
\tOptions:\n\
\t\t-f --file <configfile>\n\
\t\t\tread configuration from file (default: /etc/bagd.vhosts)\n\n\
\t\t-h --help\n\
\t\t\toutput this help text and exit\n\n\
\t\t-v --version\n\
\t\t\toutput version number and exit\n\n\
\t\t-d --debug\n\
\t\t\tdon't go into daemon mode\n\n\
\t\t-p --port <port>\n\
\t\t\tTCP port to bind to\n\n\
\t\t-b --binary --bagd </path/to/bagd>\n\
\t\t\tPath to the bagd binary\n\n\
\t\t-6 --ipv6\n\
\t\t\tuse IPv6\n\n\
"
static void help(const char*name)
{
        fprintf(stderr, PACKAGE " version " VERSION "\nUsage: %s [options]\n" HELP,name);
}

/*parse commandline*/
static int godaemon=1;
static char sopt[]="hvf:dp:b:6";
static struct option lopt[]={
        {"help",0,0,'h'},
        {"version",0,0,'v'},
        {"file",1,0,'f'},
        {"debug",0,0,'d'},
        {"port",1,0,'p'},
        {"binary",1,0,'b'},
        {"bagd",1,0,'b'},
        {"ipv6",0,0,'6'},
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

                        case 'p':port=atoi(optarg);break;

                        case 'f':vhostfile=optarg;break;

                        case 'b':bagd=optarg;break;

                        default:help(argv0);exit(1);break;
                }
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


/*standard signal handler*/
static void bagdsighandler(int sig)
{
        switch(sig){
                case SIGCHLD:{
                        int stat;
                        char*type="?unknown event?";
                        pid_t pid;
                        pid=waitpid(-1,&stat,WNOHANG|WUNTRACED);

                        if(WIFEXITED(stat))type="exited normally";
                        if(WIFSIGNALED(stat))type="killed by signal";
                        log(LOG_DEBUG,"SIGCHLD: pid=%i status=%i signal=%i %s",(int)pid,
                                WEXITSTATUS(stat),WTERMSIG(stat),type);
                        break;
                }
                default:
                        log(LOG_INFO,"got signal %i, exiting",sig);
                        exit(0);
                        break;
        }
}

/*VHost Logic*/

struct s_config{
        char*vhost,*db;
        struct s_config *next,*prev;
}*config=0;

void addconfig(char*line,int ln)
{
        int i,p=-1;
        struct s_config *c;

        /*parse*/
        for(i=0;line[i];i++)
                if(line[i]==' '||line[i]=='\t'){
                        p=i+1;
                        line[i]=0;
                        break;
                }
        if(p<0||line[0]==0||line[p]==0){
                fprintf(stderr,"Syntax error in config file line %i.\n",ln);
                exit(1);
        }

        /*add to list*/
        c=malloc(sizeof(struct s_config));
        c->vhost=line;
        c->db=line+p;
        if(config){
                c->next=config;
                c->prev=config->prev;
                config->prev=c;
                c->prev->next=c;
        }else{
                c->next=c->prev=config=c;
        }
}

void readconfig()
{
        /*open file*/
        int fd,i,j,ln;
        off_t len;
        char*buf;
        
        fd=open(vhostfile,O_RDONLY);
        if(fd<0){
                fprintf(stderr,"Error: cannot open config file %s: %s.\n",vhostfile,strerror(errno));
                exit(1);
        }

        /*read file*/
        len=lseek(fd,0,SEEK_END);
        lseek(fd,0,SEEK_SET);

        buf=malloc(len+1);
        if(!buf){
                fprintf(stderr,"Error: cannot allocate enough memory for config file.\n");
                exit(1);
        }
        read(fd,buf,len);
        buf[len]=0;
        
        close(fd);

        /*parse file*/
        /*i - search for line end, j - beginning of the line*/
        for(ln=i=j=0;i<=len;i++){
                if(buf[i]=='\n'||buf[i]==0){
                        buf[i]=0;
                        ln++;
                        if(buf[j]!='#'&&buf[j]!=0)addconfig(buf+j,ln);
                        j=i+1;
                }
        }

        /*check config*/
        if(!config){
                fprintf(stderr,"Error: empty configuration, exiting.\n");
                exit(1);
        }
}

void vhostchild(int fd)
{
        pid_t pid;
        int i,j;
        char buf[1024];
        pid=fork();
        if(pid<0){
                log(LOG_WARNING,"Cannot fork: %s.",strerror(errno));
                write(fd,"-0 sorry\n",9);
                close(fd);
                return;
        }
        if(pid){
                /*parent*/
                close(fd);
        }else{
                /*child*/
                struct s_config*tc;
                char*db=0,*hello;
                close(sock);
                if(fd!=0){
                        close(0);
                        if(dup2(fd,0)<0)exit(1);
                        close(fd);
                }
                fcntl(0,F_SETFD,0);
                
                /*say hello*/
                hello="+0 vhost-" PACKAGE " " VERSION "\n";
                write(0,hello,strlen(hello));
                
                /*get vhost...*/
                buf[0]=0;
                for(i=0;i<1023;i++){
                        j=read(0,buf+i,1);
                        if(j<1){
                                write(0,"-0 protocol error or network problem\n",37);
                                exit(1);
                        }
                        buf[i+1]=0;
                        /*null out \r, but wait for \n*/
                        if(buf[i]=='\r')buf[i]=0;
                        if(buf[i]=='\n'){
                                buf[i]=0;
                                break;
                        }
                }
                if(strncmp("0 vhost ",buf,8)){
                        write(0,"-0 expected vhost command\n",26);
                        exit(1);
                }
                /*search vhost...*/
                tc=config;
                do{
                        if(!strcmp(tc->vhost,buf+8)){
                                db=tc->db;
                                break;
                        }
                        tc=tc->next;
                }while(tc!=config);
                if(!db){
                        write(0,"-0 unknown virtual host\n",24);
                        exit(1);
                }
                /*call vhost...*/
                execlp(bagd,bagd,"-m","--",db,0);
                /*this shouldn't happen, apologize...*/
                log(LOG_WARNING,"Cannot execute bagd: %s.",strerror(errno));
                write(0,"-0 sorry\n",9);
                exit(1);
        }
}

int main(int argc,char**argv)
{
        struct sigaction sa;

        handleoptions(argc,argv);

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

        readconfig();


        /*init socket*/
        if(ipv6){
                /*tcp6*/
                struct sockaddr_in6 si;
                int sopt;
                sock=socket(PF_INET6,SOCK_STREAM,0);
                if(sock==-1){
                        log(LOG_ERR,"Unable to allocate TCPv6 socket: %s",strerror(errno));
                        exit(1);
               }
               if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&sopt,sizeof(sopt))==-1)
                       log(LOG_ERR,"Unable to set reuse socket option on.");
               si.sin6_family=AF_INET6;
               si.sin6_addr=in6addr_any;
               si.sin6_port=htons(port);
               if(bind(sock,&si,sizeof(si))==-1){
                       log(LOG_ERR,"Unable to bind TCPv6 socket: %s",strerror(errno));
                       close(sock);
                       exit(1);
               }
        }else{
                /*tcp*/
                struct sockaddr_in si;
                int sopt;
                sock=socket(PF_INET,SOCK_STREAM,0);
                if(sock==-1){
                        log(LOG_ERR,"Unable to allocate TCP socket: %s",strerror(errno));
                        exit(1);
                }
                sopt=1;
                if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&sopt,sizeof(sopt))==-1)
                        log(LOG_ERR,"Unable to set reuse socket option.");
                si.sin_family=AF_INET;
                si.sin_addr.s_addr=htonl(INADDR_ANY);
                si.sin_port=htons(port);
                if(bind(sock,&si,sizeof(si))==-1){
                        log(LOG_ERR,"Unable to bind TCP socket: %s",strerror(errno));
                        close(sock);
                        exit(1);
                }
        }
        if(listen(sock,5)==-1){
                log(LOG_WARNING,"Unable to set TCP socket to listen mode: %s",strerror(errno));
                close(sock);
                exit(1);
        }
        log(LOG_INFO,"Successfully initialized TCP socket. Port=%i FD=%i.",port,sock);


        /*go daemons go...*/
        if(godaemon)daemonize();


        for(;;){
                int fd;
                fd=accept(sock,0,0);
                if(fd>-1)vhostchild(fd);
        }

        return 0;
}