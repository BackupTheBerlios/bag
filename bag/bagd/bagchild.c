/***************************************************************************
                          bagchild.c  -  description
                             -------------------
    begin                : Thu Oct 3 2002
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
#include "bagchild.h"
#include "bagd.h"
#include "log.h"
#include <libpq-fe.h>

#include <signal.h>
#include <stdio.h>
#include <string.h>


#include "../config.h"

#include "sql.h"

PGconn *bc_con=0;
struct s_sockethandler*bc_hdl=0;

#define E_LINETOOLONG "-0 line too long\n"
#define E_ILLEGALSEQUENCE "-0 illegal backlash sequence\n"
#define E_BLOBLEN "-0 illegal value for blob length\n"
#define E_NOBLOB "-0 this function expects no blob\n"
#define E_READBLOB "-0 unable to read complete blob\n"
#define E_NOCOMMAND "-0 no such command or command not allowed in current mode\n"

int processline(struct s_linehandler*slh)
{
        /*read a line*/
        char linebuf[1025],**argv=0,*eptr=0;
        int llen=-1,r,i,j,argc;
        unsigned long bloblen=0;
        void*blob=0;
        do{
                llen++;
                if(llen==1025){
                        bc_hdl->sockwriter(bc_hdl,E_LINETOOLONG,strlen(E_LINETOOLONG));
                        return -1;
                }
                r=bc_hdl->sockreader(bc_hdl,linebuf+llen,1);
                if(r<0)return -1;
        }while(linebuf[llen]!='\n');
        linebuf[llen]=0;
        /*compensate \r*/
        if(linebuf[llen-1]=='\r'){
                llen--;
                linebuf[llen]=0;
        }

        /*count spaces*/
        for(argc=i=0;linebuf[i];i++)if(linebuf[i]==' ')argc++;
        if(argc==0){
                bc_hdl->sockwriter(bc_hdl,E_NOCOMMAND,strlen(E_NOCOMMAND));
                return -1;
        }
        /*parse the line*/
         /*first hunk is left out, since it is the bloblength*/
        argv=malloc(sizeof(char**)*argc);
        if(!argv){
                bc_hdl->sockwriter(bc_hdl,E_ALLOCATION,strlen(E_ALLOCATION));
                return -1;
        }
        r=strlen(linebuf);

        for(j=i=0;i<r;i++)
                if(linebuf[i]==' '){
                        argv[j++]=linebuf+i+1;
                        linebuf[i]=0;
                }

        /*get blob*/
        bloblen=strtoul(linebuf,&eptr,10);
        if(*eptr!=0){
                free(argv);
                bc_hdl->sockwriter(bc_hdl,E_BLOBLEN,strlen(E_BLOBLEN));
                return -1;
        }
        if(bloblen){
                blob=malloc(bloblen);
                if(!blob){
                        free(argv);
                        bc_hdl->sockwriter(bc_hdl,E_ALLOCATION,strlen(E_ALLOCATION));
                        return -1;
                }
                r=bc_hdl->sockreader(bc_hdl,blob,bloblen);
                if(r<bloblen){
                        /*normally this should not happen, since the reader is required to
                          clean everything up and exit*/
                        free(blob);
                        free(argv);
                        bc_hdl->sockwriter(bc_hdl,E_READBLOB,strlen(E_READBLOB));
                        return -1;
                }
        }
        
        /*call*/
        for(i=0;slh[i].command;i++)
                if(!strcmp(argv[0],slh[i].command)){
                        if(slh[i].hasblob==0 && bloblen>0){
                                free(argv);
                                if(blob)free(blob);
                                bc_hdl->sockwriter(bc_hdl,E_NOBLOB,strlen(E_NOBLOB));
                                return -1;
                        }
                        slh[i].linehandler(argc,argv,bloblen,blob);
                        free(argv);
                        if(blob)free(blob);
                        return i;
                }
                
        /*free, return*/
        bc_hdl->sockwriter(bc_hdl,E_NOCOMMAND,strlen(E_NOCOMMAND));
        free(argv);
        if(blob)free(blob);
        return -1;
}


static void bagdchildsighandler(int sig)
{
        log(LOG_WARNING,"Bagd child %i catched signal %i\n",getpid(),sig);
        switch(sig){
                case SIGPIPE:case SIGINT:case SIGTERM:case SIGQUIT:
                        PQfinish(bc_con);
                        bc_hdl->sockcloser(bc_hdl);
                        exit(1);
                        break;
                case SIGHUP:
                        /*reconfigure*/
                        break;
                default:
                        /*ignore the rest*/
                        break;
        }
}

static void writehello()
{
        /*FIXME: some way is needed to extract the FQDN instead of the
          simple hostname*/
        char buf[1024],host[256];
        if(gethostname(host,255))strcpy(host,"(unknown)");
        sprintf(buf,"+ " PACKAGE " " VERSION " on %s\n",host);
        bc_hdl->sockwriter(bc_hdl,buf,strlen(buf));
}

static void protocollerror(struct s_sockethandler*hdl)
{
        hdl->sockwriter(hdl,"- Protocoll error\n",18);
        PQfinish(bc_con);
        exit(1);
}

void closechild()
{
        /*close connection*/
        bc_hdl->sockcloser(bc_hdl);

        PQfinish(bc_con);

        exit(0);
}


void bagchild(struct s_sockethandler*hdl,const char*dbstring)
{
        PGresult *res;
        struct sigaction sa;
        bc_hdl=hdl;
        
        /*init signals*/
        sa.sa_sigaction=0;
        sa.sa_handler=bagdchildsighandler;
        sa.sa_flags=SA_NOMASK|SA_RESTART;
        sigaction(SIGHUP,&sa,0);
        sigaction(SIGTERM,&sa,0);
        sigaction(SIGQUIT,&sa,0);
        sigaction(SIGCHLD,&sa,0);
        sigaction(SIGPIPE,&sa,0);
        sigaction(SIGINT,&sa,0);

        /*open DB*/
        bc_con=PQconnectdb(connectstring);
        if(PQstatus(bc_con)==CONNECTION_BAD){
                log(LOG_ERR,"DB Connection Error: %s\n",PQerrorMessage(bc_con));
                PQfinish(bc_con);
                exit(1);
        }

        PQclear(PQexec(bc_con,SQL_INITSESSION));


        /*init connection*/
        writehello();

        if(authenticate()){
                /*do normal work*/
                mainmode();
        }
        

        /*finish child process*/
        closechild();
        return; /*return to spawnchild and exit there in case of error (unlikely)*/
}
