/***************************************************************************
                          bc_main.c  -  description
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

#include "../config.h"
#include "log.h"
#include "bagchild.h"
#include "bc_auth.h"

#include <stdio.h>
#include <string.h>

static void hello_handler(int argc,char**argv,int bloblen,void*blob)
{
        char buf[32];
        sprintf(buf,"+%i hello.\n",bloblen);
        bc_hdl->sockwriter(bc_hdl,buf,strlen(buf));
        bc_hdl->sockwriter(bc_hdl,blob,bloblen);
}

static int doquit=0;

void quit_handler(int argc,char**argv,int bloblen,void*blob)
{
        bc_hdl->sockwriter(bc_hdl,"+0 quitting\n",12);
        closechild();
}



static struct s_linehandler mainlinehandler[]={
        {"hello",1,hello_handler},
        {"quit",0,quit_handler},
        {0,0,0}
};


void mainmode()
{
        log(LOG_DEBUG,"reached main mode...\n");
        while(!doquit){
                processline(mainlinehandler);
        }
}
