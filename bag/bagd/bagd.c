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
#include <libpq-fe.h>
#include "sql.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "bagstream.h"
#include "bagssl.h"

static char *connectstring;

static int tosyslog=0,nsockets=0,*sockets=0,*isssl=0;
static char **socketpaths=0;


void log(const char*format,...)
{
        va_list arg;
        va_start(arg,format);
        if(tosyslog){
                char buf[1024];
                vsnprintf(buf,sizeof(buf),format,arg);
                syslog(LOG_INFO,"%s",buf);
        }else vfprintf(stderr,format,arg);
        va_end(arg);
}


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

static void config()
{
        PGconn *con;
        int i;
        char*s;
        PGresult *res;
        con=PQconnectdb(connectstring);
        if(PQstatus(con)==CONNECTION_BAD){
                log("Connection Error: %s\n",PQerrorMessage(con));
                PQfinish(con);
                exit(1);
        }

        res=PQexec(con,SQL_GETOPTIONS);
        if(!res || PQresultStatus(res) != PGRES_COMMAND_OK){
                log("Unable to get configuration: %s\n",PQerrorMessage(con));
                if(res)PQclear(res);
                PQfinish(con);
                exit(1);
        }

        nsockets=PQntuples(res);

        sockets=malloc(sizeof(int)*nsockets);
        isssl=malloc(sizeof(int)*nsockets);
        socketpaths=malloc(sizeof(char*)*nsockets);

        for(i=0;i<nsockets;i++){
                char*x;
                int t;/*0=error, 1=TCP, 2=AF_UNIX, 3=TCPv6 ...*/
                s=PQgetvalue(res,i,1);
                sockets[i]=-1;t=0;
                socketpaths[i]=0;
                if(!strncmp("tcp:",s,4)){t=1;x=s+4;isssl[i]=0;}else
                if(!strncmp("ssl:",s,4)){t=1;x=s+4;isssl[i]=1;}else
                if(!strncmp("tcp6:",s,5)){t=3;x=s+4;isssl[i]=0;}else
                if(!strncmp("ssl6:",s,5)){t=3;x=s+4;isssl[i]=1;}else
                if(!strncmp("local:",s,6)){t=2;x=s+6;isssl[i]=0;}else
                if(!strncmp("unix:",s,5)){t=2;x=s+5;isssl[i]=0;}else
                if(!strncmp("slocal:",s,7)){t=2;x=s+7;isssl[i]=1;}else
                if(!strncmp("sunix:",s,6)){t=2;x=s+6;isssl[i]=1;}
                else{
                        log("Failed opening socket %s, unknown type.\n",s);
                        continue;
                }
                switch(t){
                        case 1:{/*tcp*/
                                struct sockaddr_in si;
                                sockets[i]=socket(PF_INET,SOCK_STREAM,0);
                                if(sockets[i]==-1){
                                        log("Unable to allocate TCP socket %s: %s\n",s,strerror(errno));
                                        continue;
                                }
                                si.sin_family=AF_INET;
                                si.sin_addr.s_addr=htonl(INADDR_ANY);
                                si.sin_port=htons(atoi(x));
                                if(bind(sockets[i],&si,sizeof(si))==-1){
                                        log("Unable to bind TCP socket %s: %s\n",s,strerror(errno));
                                        close(sockets[i]);
                                        sockets[i]=-1;
                                        continue;
                                }
                                if(listen(sockets[i],5)==-1){
                                        log("Unable to set TCP socket %s to listen mode: %s\n",s,strerror(errno));
                                        close(sockets[i]);
                                        sockets[i]=-1;
                                        continue;
                                }
                                log("Successfully initialized TCP socket %s.\n",s);
                                break;
                        }
                        case 3:{/*tcp6*/
                                struct sockaddr_in6 si;
                                sockets[i]=socket(PF_INET6,SOCK_STREAM,0);
                                if(sockets[i]==-1){
                                        log("Unable to allocate TCPv6 socket %s: %s\n",s,strerror(errno));
                                        continue;
                                }
                                si.sin6_family=AF_INET6;
                                si.sin6_addr=in6addr_any;
                                si.sin6_port=htons(atoi(x));
                                if(bind(sockets[i],&si,sizeof(si))==-1){
                                        log("Unable to bind TCPv6 socket %s: %s\n",s,strerror(errno));
                                        close(sockets[i]);
                                        sockets[i]=-1;
                                        continue;
                                }
                                if(listen(sockets[i],5)==-1){
                                        log("Unable to set TCPv6 socket %s to listen mode: %s\n",s,strerror(errno));
                                        close(sockets[i]);
                                        sockets[i]=-1;
                                        continue;
                                }
                                log("Successfully initialized TCPv6 socket %s.\n",s);
                                break;
                        }
                        case 2:{/*local*/
                                struct sockaddr_un su;
                                sockets[i]=socket(PF_UNIX,SOCK_STREAM,0);
                                if(sockets[i]==-1){
                                        log("Unable to allocate AF_UNIX socket %s: %s\n",s,strerror(errno));
                                        continue;
                                }
                                su.sun_family=AF_UNIX;
                                strcpy(su.sun_path,x);
                                unlink(x);
                                if(bind(sockets[i],&su,sizeof(su))==-1){
                                        log("Unable to bind AF_UNIX socket %s: %s\n",s,strerror(errno));
                                        close(sockets[i]);
                                        sockets[i]=-1;
                                        continue;
                                }
                                if(listen(sockets[i],5)==-1){
                                        log("Unable to set AF_UNIX socket %s to listen mode: %s\n",s,strerror(errno));
                                        close(sockets[i]);
                                        sockets[i]=-1;
                                        unlink(x);
                                        continue;
                                }
                                log("Successfully initialized socket %s.\n",s);
                                socketpaths[i]=malloc(strlen(x)+1);
                                strcpy(socketpaths[i],x);
                                break;
                        }
                        default:/*error/unknown*/
                                log("Unknown socket type or internal error: %s.\n",s);
                                continue;
                                break;
                }
        }
        
        PQclear(res);
        PQfinish(con);
}

static void sighandler(int sig)
{
        switch(sig){
                case SIGHUP:
                        deconfig();
                        config();
                        break;
                case SIGCHLD:
                        wait();
                        break;
                default:
                        deconfig();
                        exit(0);
                        break;
        }
}


static void spawnchild(int fd,int usessl)
{
        pid_t pid;
        pid=fork();
        if(pid<0){
                log("Unable to spawn another process: %s.\n",strerror(errno));
                close(fd);
                return;
        }else
        if(pid==0){/*child*/
                struct s_sockethandler*sh;
                if(usessl)sh=newsslhandler(fd);
                else sh=newstreamhandler(fd);
                bagchild(sh,connectstring);
                exit(0);
        }else{/*parent*/
                return;
        }
}

void daemonize()
{
        close(0);close(1);close(2);
        if(fork())exit(0);
        tosyslog=1;
        chdir("/");
}

void main(int argc,char**argv)
{
        int i,l;
        if(argc<2||argc>3){
                fprintf(stderr,"Usage: %s \"connection string\" [-d]\n",argv[0]);
                exit(1);
        }
        if(argc<3)daemonize();
        else{
                if(strcmp(argv[2],"-d")){
                        log("expecting -d as 2nd parameter.\n");
                        exit(1);
                }
        }
        connectstring=malloc((l=strlen(argv[1]))+1);
        strcpy(connectstring,argv[1]);
        /*hide connection string, it probably contains a password*/
        for(i=0;i<l;i++)argv[1][i]=0;
        
        config();

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
                        log("Fatal error: no listening sockets remaining, exiting.\n");
                        exit(1);
                }
                i=select(m+1,&rd,0,0,0);
                if(i==-1){
                        int e=errno;
                        if(e==EINTR)continue;
                        log("Fatal error on select: %s, exiting.\n",strerror(e));
                        exit(1);
                }
                for(i=0;i<nsockets;i++)
                        if(sockets[i]>=0 && FD_ISSET(sockets[i],&rd)){
                                int fd=accept(sockets[i],0,0);
                                if(fd<0){
                                        int e=errno;
                                        if(e==EAGAIN||e==EWOULDBLOCK)continue;
                                        log("Internal error on accept(%i,0,0): %s, exiting.\n",fd,strerror(e));
                                        exit(1);
                                }
                                spawnchild(fd,isssl[i]);
                        }

        }
}
