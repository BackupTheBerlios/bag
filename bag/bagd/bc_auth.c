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

char*username=0;


static void auth_handler(int argc,char**argv,int bloblen,void*blob)
{
        /*workaround if the compiler doesn't optimize:*/
        if(username)return;

        /*check arguments*/
        
        /*select by username*/

        
        /*try certificate auth, yet unimplemented*/
        /*if(bc_hdl->sockauth)bc_hdl->sockauth(...);*/

        /*try password auth*/
}



static struct s_linehandler authlinehandler[]={
        {"auth",0,auth_handler},
        {0,0,0}
};

int authenticate()
{
        if(processline(authlinehandler)==-1 &&
           processline(authlinehandler)==-1 &&
           processline(authlinehandler)==-1){
                return 0;
        }else
                if(username)return 1;
                else return 0;
}
