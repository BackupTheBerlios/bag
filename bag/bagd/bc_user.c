/***************************************************************************
                          bc_user.c  -  description
                             -------------------
    begin                : Sat Dec 28 2002
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
#include "bc_user.h"
#include "bc_auth.h"
#include "query.h"
#include "defines.h"
#include "sql.h"
#include "bagd.h"
#include "log.h"

#include "../libbag/dynbuf.h"

#include <stdlib.h>

void config_handler(int argc,char**argv,int bloblen,void*blob)
{
        dbResult*res;
        int i,l,l2,n;
        char*blob2;
        /*check ACL*/
        if(!checkGlobalACL(ACL_CONFIG)){
                bc_hdl->sockwriter(bc_hdl,E_PERM,strlen(E_PERM));
                return;
        }
        /*invalid argc*/
        if(argc>3){
                bc_hdl->sockwriter(bc_hdl,E_SYNTAX,strlen(E_SYNTAX));
                return;
        }
        /*query*/
        if(argc==1){
                /*query*/
                res=query(bc_con,SQL_GETOPTIONS,servername);
                /*create blob*/
                if(!res){
                        bc_hdl->sockwriter(bc_hdl,E_DATABASE,strlen(E_DATABASE));
                        return;
                }
                l=dbNumRows(res);
                n=dbGetFieldIndex(res,"okey");
                for(l2=i=0;i<l;i++){
                        l2+=strlen(dbGetString(res,i,n))+1;
                }
                blob2=malloc(l2+64);/*64 should be enough for the text*/
                if(!blob2){
                        dbFree(res);
                        bc_hdl->sockwriter(bc_hdl,E_ALLOCATION,strlen(E_ALLOCATION));
                        return;
                }
                *blob2=0;
                sprintf(blob2,"+%i Options:\n",l2);
                for(i=0;i<l;i++)
                        strcat(strcat(blob2,strchr(dbGetString(res,i,n),':')+1),"\n");
                dbFree(res);
                /*send*/
                bc_hdl->sockwriter(bc_hdl,blob2,strlen(blob2));
                /*free*/
                free(blob2);
                return;
        }
        if(argc==2){
                res=query(bc_con,SQL_GETOPTION,servername,argv[1]);
                if(!res){
                        bc_hdl->sockwriter(bc_hdl,E_DATABASE,strlen(E_DATABASE));
                        return;
                }
                if(dbNumRows(res)){
                        char buf[64];
                        sprintf(buf,"+0 %s\n",dbGetStringByname(res,0,"oval"));
                        bc_hdl->sockwriter(bc_hdl,buf,strlen(buf));
                }else{
                        bc_hdl->sockwriter(bc_hdl,E_NOOPT,strlen(E_NOOPT));
                }
                dbFree(res);
                return;
        }
        /*set*/
        if(argc==3){
                /*FIXME: this is not transaction safe, unfortunately I know no safe way*/
                /*try update*/
                res=query(bc_con,SQL_UPDATEOPTION,argv[2],servername,argv[1]);
                if(!res){
                        bc_hdl->sockwriter(bc_hdl,E_DATABASE,strlen(E_DATABASE));
                        return;
                }
                if(dbAffectedRows(res)<=0){
                        dbFree(res);
                        /*try insert*/
                        res=query(bc_con,SQL_INSERTOPTION,argv[2],servername,argv[1]);
                }
                if(!dbUpdateOK(res)){
                        bc_hdl->sockwriter(bc_hdl,E_DATABASE,strlen(E_DATABASE));
                }else{
                        bc_hdl->sockwriter(bc_hdl,E_OK,strlen(E_OK));
                }
                if(res)dbFree(res);
                return;
        }
}

void getusers_handler(int argc,char**argv,int bloblen,void*blob)
{
        dbResult*res;
        int i,l;
        char rsp[64];
        struct dynbuf_s*buf;
        if(!checkGlobalACL(ACL_ADMUSER)){
                bc_hdl->sockwriter(bc_hdl,E_PERM,strlen(E_PERM));
                return;
        }
        res=query(bc_con,SQL_GETUSERS);
        if(!res){
                bc_hdl->sockwriter(bc_hdl,E_DATABASE,strlen(E_DATABASE));
                return;
        }
        buf=dynbuf_new();
        if(!buf){
                bc_hdl->sockwriter(bc_hdl,E_ALLOCATION,strlen(E_ALLOCATION));
                dbFree(res);
                return;
        }
        l=dbNumRows(res);
        for(i=0;i<l;i++){
                dynbuf_addstr(buf,dbGetStringByname(res,i,"usrname"));
                dynbuf_addstr(buf,"\n");
        }
        dbFree(res);
        sprintf(rsp,"+%i Users:\n",buf->len);
        bc_hdl->sockwriter(bc_hdl,rsp,strlen(rsp));
        bc_hdl->sockwriter(bc_hdl,buf->buffer,buf->len);
        dynbuf_free(buf);
}

void getuseracl_handler(int argc,char**argv,int bloblen,void*blob);
void setuseracl_handler(int argc,char**argv,int bloblen,void*blob);
void createuser_handler(int argc,char**argv,int bloblen,void*blob);
void dropuser_handler(int argc,char**argv,int bloblen,void*blob);
void setpassword_handler(int argc,char**argv,int bloblen,void*blob);
void disableuser_handler(int argc,char**argv,int bloblen,void*blob);
void enableuser_handler(int argc,char**argv,int bloblen,void*blob);

int checkGlobalACL(char cap)
{
        dbResult *res;
        char *acl;

        res=query(bc_con,SQL_GETMAINACL,username);

        if(!res)return 0;
        if(dbNumRows(res)!=1){
                dbFree(res);
                return 0;
        }

        acl=dbGetString(res,0,0);
        if(!acl)acl="";

        if(strchr(acl,cap)||strchr(acl,'A')){
                dbFree(res);
                return 1;
        }else{
                dbFree(res);
                return 0;
        }
}
