/***************************************************************************
                          bc_main.c  -  child - main mode
                             -------------------
    begin                : Wed Oct 16 2002
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

#include "bc_main.h"
#include "bc_user.h"

#include "../config.h"
#include "log.h"
#include "bagchild.h"
#include "bc_auth.h"
#include "defines.h"

#include "../libbag/dynbuf.h"

#include <stdio.h>
#include <string.h>


static void echo_handler(int argc,char**argv,int bloblen,void*blob)
{
        char buf[64]; /*64 byte is plenty for this:*/
        sprintf(buf,"+0 echo-reply %i\n",bloblen);
        bc_hdl->sockwriter(bc_hdl,buf,strlen(buf));
}

static int doquit=0;

void quit_handler(int argc,char**argv,int bloblen,void*blob)
{
        bc_hdl->sockwriter(bc_hdl,"+0 quitting\n",12);
        closechild();
}

static void packlevel_handler(int argc,char**argv,int bloblen,void*blob)
{
        if(argc==1){
                bc_hdl->sockwriter(bc_hdl,"+0 0\n",5);
        }else if(argc==2){
                if(!strcmp("0",argv[1])){
                        bc_hdl->sockwriter(bc_hdl,"+0 packlevel=0\n",15);
                }else{
                        bc_hdl->sockwriter(bc_hdl,"-0 illegal packlevel\n",21);
                }
        }else/*argc>2||argc<1*/{
                bc_hdl->sockwriter(bc_hdl,E_SYNTAX,strlen(E_SYNTAX));
        }
}

static void help_handler(int argc,char**argv,int bloblen,void*blob);


static struct s_linehandler mainlinehandler[]={
        /*bc_main*/
        {"echo",1,echo_handler},
        {"quit",0,quit_handler},
        {"packlevel",0,packlevel_handler},
        {"help",0,help_handler},
        /*bc_user*/
        {"config",0,config_handler},
        {"getusers",0,getusers_handler},
        /*EOL*/
        {0,0,0}
};

static void help_handler(int argc,char**argv,int bloblen,void*blob)
{
        struct dynbuf_s *b;
        char fl[32];
        int i,bd;
        b=dynbuf_new();
        if(!b){
                bc_hdl->sockwriter(bc_hdl,"-0 allocation problem\n",22);
                return;
        }
        dynbuf_addstr(b,PACKAGE " " VERSION "\nFUNCTION HAS-BLOB\n");
        for(i=0;mainlinehandler[i].command;i++){
                bd=dynbuf_addstr(b,mainlinehandler[i].command);
                if(mainlinehandler[i].hasblob)
                        bd|=dynbuf_addstr(b," yes\n");
                else
                        bd|=dynbuf_addstr(b," no\n");
                if(bd){
                        bc_hdl->sockwriter(bc_hdl,"-0 allocation problem\n",22);
                        dynbuf_free(b);
                        return;
                }
        }
        sprintf(fl,"+%i\n",b->len);
        bc_hdl->sockwriter(bc_hdl,fl,strlen(fl));
        bc_hdl->sockwriter(bc_hdl,b->buffer,b->len);
        return;
}


void mainmode()
{
        log(LOG_DEBUG,"reached main mode...");
        while(!doquit){
                processline(mainlinehandler);
        }
}
