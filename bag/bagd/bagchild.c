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
#include "log.h"
#include <libpq-fe.h>

#include <signal.h>
#include <stdio.h>
#include <string.h>


#include "../config.h"

PGconn *bc_con=0;
struct s_sockethandler*bc_hdl=0;



int processline(struct s_linehandler*slh)
{
        /*FIXME: this could be better buffered*/
        /*read a line*/
        char linebuf[1025];
        int llen=-1,r;
        do{
                llen++;
                if(llen==1025)return -1;
                r=bc_hdl->sockreader(bc_hdl,linebuf+llen,1)
                if(r<0)return r;
        }while(linebuf[llen]!='\n');
        linebuf[llen]=0;

        /*parse the line*/
                
}


static void bagdchildsighandler(int sig)
{
        switch(sig){
                case SIGPIPE:case SIGINT:case SIGTERM:case SIGQUIT:
                        PQfinish(bc_con);
                        bc_hdl->socketcloser(bc_hdl);
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
        PQfinish(con);
        exit(1);
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
                log(LOG_ERR,"DB Connection Error: %s\n",PQerrorMessage(con));
                PQfinish(bc_con);
                exit(1);
        }

        PQclear(PQexec(con,SQL_INITSESSION));


        /*init connection*/
        writehello();

        if(authenticate()){
                /*do normal work*/
        }
        

        /*close connection*/
        hdl->sockcloser(bc_hdl);
        
        PQfinish(con);
        return; /*return to spawnchild and exit there*/
}
