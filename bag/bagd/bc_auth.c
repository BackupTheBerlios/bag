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
#include "bc_main.h"
#include "md5.h"

#include <security/pam_appl.h>

#include <libpq-fe.h>
#include "sql.h"
#include "query.h"
#include "log.h"

/*FIXME: username is allocated but not freed*/
char*username=0;


#define E_SYNTAX "-0 auth: bad syntax\n"
#define E_NOUSER "-0 auth: no such user\n"
#define E_NOPERM "-0 auth: user locked, wrong password, or wrong certificate\n"
#define E_STORAGE "-0 internal storage error, please contact the server admin\n"

#define E_AUTHOK "+0 auth: ok\n"

static int fpamconv(int num,const struct pam_message **msg,struct pam_response **rsp,void*appdt)
{
        int i;
        char*tmp;
        *rsp=malloc(num*sizeof(struct pam_response));
        for(i=0;i<num;i++){
                if((msg[i])->msg_style==PAM_PROMPT_ECHO_OFF){
                        tmp=malloc(strlen(appdt)+1);
                        strcpy(tmp,appdt);
                        (*rsp)[i].resp=tmp;
                        (*rsp)[i].resp_retcode=0;
                }else{
                        (*rsp)[i].resp=NULL;
                        (*rsp)[i].resp_retcode=0;
                }
        }
        return 0;
}

static int pamauth(const char*user,struct pam_conv*conv)
{
    pam_handle_t *pamh=NULL;
    int retval;

    /*FIXME: service ("bagd") should be configurable*/
    retval = pam_start("bagd", user, conv, &pamh);

    if (retval == PAM_SUCCESS)
        retval = pam_authenticate(pamh, 0);    /* is user really user? */

    if (retval == PAM_SUCCESS)
        retval = pam_acct_mgmt(pamh, 0);       /* permitted access? */

    /* This is where we have been authorized or not. */

    if (pam_end(pamh,retval) != PAM_SUCCESS) {     /* close Linux-PAM */
        pamh = NULL;
        fprintf(stderr, "ERROR failed to release authenticator\n");
        exit(1);
    }

    return retval == PAM_SUCCESS ;       /* indicate success */
}




static void auth_handler(int argc,char**argv,int bloblen,void*blob)
{
        char*uname;
        PGresult *res;
        int method=0;
        
        log(LOG_DEBUG,"authenticating\n");

        /*check arguments*/
        if(argc<2||argc>3){
                bc_hdl->sockwriter(bc_hdl,E_SYNTAX,strlen(E_SYNTAX));
                return;
        }
        
        /*select by username*/
        uname=argv[1];
        res=PQexec(bc_con,query(SQL_GETUSER,uname));

        if(PQntuples(res)<1){
                bc_hdl->sockwriter(bc_hdl,E_NOUSER,strlen(E_NOUSER));
                PQclear(res);
                return;
        }
        if(PQgetisnull(res,0,PQfnumber(res,"usrauthmethod"))){
                bc_hdl->sockwriter(bc_hdl,E_STORAGE,strlen(E_STORAGE));
                PQclear(res);
                return;
        }
        method=atoi(PQgetvalue(res,0,PQfnumber(res,"usrauthmethod")));

        /*check whether locked*/
        if(method&AUTH_LOCKED){
                PQclear(res);
                bc_hdl->sockwriter(bc_hdl,E_NOPERM,strlen(E_NOPERM));
                return;
        }
        
        /*try password auth*/
        if(argc==3){
                if(method&AUTH_PAM){//PAM authentification requested
                        struct pam_conv pamconv = {
                                fpamconv,
                                NULL
                        };
                        pamconv.appdata_ptr=argv[2];
                        if(pamauth(uname,&pamconv)){
                                username=strdup(uname);
                                PQclear(res);
                                bc_hdl->sockwriter(bc_hdl,E_AUTHOK,strlen(E_AUTHOK));
                                return;
                        }
                }
                if(method&AUTH_INTERN){//internal MD5
                        char mdb[33],*pwd;
                        
                        md5_hex_buffer(argv[2],strlen(argv[2]),mdb);
                        if(!PQgetisnull(res,0,PQfnumber(res,"usrpasswd"))){
                                pwd=PQgetvalue(res,0,PQfnumber(res,"usrpasswd"));
                                if(!strcmp(mdb,pwd)){
                                        username=strdup(uname);
                                        PQclear(res);
                                        bc_hdl->sockwriter(bc_hdl,E_AUTHOK,strlen(E_AUTHOK));
                                        return;
                                }
                        }
                }
        }

        /*try certificate auth, yet unimplemented*/
        if((method&AUTH_SSL) && bc_hdl->sockauth){
                char*buf;
                int ret=0;
                //Fetch cert (Transaction!!)
                //ret=bc_hdl->sockauth(bc_hdl,buf);
                //free(buf);
                if(ret){
                        username=strdup(uname);
                        PQclear(res);
                        bc_hdl->sockwriter(bc_hdl,E_AUTHOK,strlen(E_AUTHOK));
                        return;
                }
                       
        }

        /*not able to authenticate*/
        PQclear(res);
        bc_hdl->sockwriter(bc_hdl,E_NOPERM,strlen(E_NOPERM));
        return;
}


static struct s_linehandler authlinehandler[]={
        {"auth",0,auth_handler},
        {"quit",0,quit_handler},/*defined in bc_main*/
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
