/***************************************************************************
                          bc_auth.c  -  description
                             -------------------
    begin                : Sat Oct 5 2002
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

#include "bagchild.h"
#include "bc_auth.h"
#include "md5.h"

#include <libpq-fe.h>
#include "sql.h"
#include "query.h"
#include "log.h"

char*username=0;


#define E_SYNTAX "-0 auth: bad syntax\n"

void auth_handler(int argc,char**argv,int bloblen,void*blob)
{
        /*workaround if the compiler doesn't optimize:*/
        log(LOG_DEBUG,"authenticating\n");

        /*check arguments*/
        if(argc<2||argc>3){
                bc_hdl->sockwriter(bc_hdl,E_SYNTAX,strlen(E_SYNTAX));
                return;
        }
        
        /*select by username*/
        /*FIXME: dummy auth*/
        username="admin";
        return;
        
        /*try certificate auth, yet unimplemented*/
        /*if(argc<3)if(bc_hdl->sockauth)bc_hdl->sockauth(...);*/

        /*try password auth*/
}



static struct s_linehandler authlinehandler[]={
        {"auth",0,auth_handler},
        {0,0,0}
};

int authenticate()
{
        if(processline(authlinehandler)<0 || !username)
                if(processline(authlinehandler)<0 || !username)
                        if(processline(authlinehandler)<0 || !username){
                                return 0;
                        }
        if(username)return 1;
        else return 0;
}
