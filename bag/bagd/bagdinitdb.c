/***************************************************************************
                          bagdinitdb.c  -  Initialize a BaG Database
                             -------------------
    begin                : Tue Aug 20 2002
    copyright            : (C) 2002 by konrad
    email                : konrad@zaphod
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
 
#define _GNU_SOURCE
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>


#include "sql.h"
#include "query.h"

#define GRANT_SELECT "select"
#define GRANT_ALL "select,insert,update,delete"

#define HELP "Usage: %s [-hvVspduwoiUPaA] \\\n\
        [--server ...] [--port ...] \\\n\
        [--dbname ...] [--user ...] [--passwd [...]] [--options ...] \\\n\
        [--initfile [...]] \\\n\
        [--inituser ...] [--initpasswd ...] \\\n\
        [--readaccess ...] [--writeaccess ...] \n\n\
        -h --help - show this help page and exit\n\
        -v --verbose - be a little bit more verbose\n\
        -V --version - show version number and exit\n\
        -s --server <server> - hostname of the DB server (localhost)\n\
        -p --port <port> - Port number of the DB server (5432)\n\
        -d --dbname <dbname> - Name of the Bag-Database to create\n\
        -u --user <dbuser> - User name to use for DB creation\n\
        -w --passwd <dbpasswd> - Password to access the DB\n\
        -o --options \"<options>\" - additional options for the DB Backend\n\
        -i --initfile [<filename>] - write a file suitable for /etc/init.d\n\
                                     (init.d_bagd.sh)\n\
        -U --inituser <username> - initial Admin-Username for this Bag (bag)\n\
        -P --initpasswd <passwd> - initial Password for the initial user (bag)\n\
        -a --readaccess <user,user...> - DB Users to have read access\n\
        -A --writeaccess <user,user...> - DB Users to have write access\n\
        -S --initsocket /<socketname> - initial Socketname (/tmp/baginit)\n"
        

const char optstring[]="hvVs:p:d:u:w::o:i:U:P::a:A:S:";

const struct option longopts[]={
        {"help",0,0,'h'},
        {"verbose",0,0,'v'},
        {"version",0,0,'V'},
        {"server",1,0,'s'},
        {"port",1,0,'p'},
        {"dbname",1,0,'d'},
        {"user",1,0,'u'},
        {"passwd",2,0,'w'},
        {"options",1,0,'o'},
        {"initfile",2,0,'i'},
        {"inituser",1,0,'U'},
        {"initpasswd",2,0,'P'},
        {"readaccess",1,0,'a'},
        {"writeaccess",1,0,'A'},
        {"initsocket",1,0,'S'},
        {0,0,0,0} };
        


void setecho(int b)
{
        struct termios t;
        tcgetattr(STDIN_FILENO,&t);
        if(b){
                t.c_lflag|=ECHO;
        }else{
                t.c_lflag&= ~ECHO;
        }
        tcsetattr(STDIN_FILENO,TCSANOW,&t);
}

void exitdb(PGconn*con)
{
        PQfinish(con);
        exit(1);
}
        
int main(int argc,char**argv)
{
        int verbose=0,askpasswd=0,opt,doinitfile=0;
        char *server="localhost",*dbname=0,*user=0,*passwd=0,*options="",*initfile="init.d_bagd.sh",
             *inituser="bag",*initpasswd="bag",*readacc=0,*writeacc=0,*port="5432",
             *initsocket="/tmp/baginit",*selfname=argv[0];
        char pwdbuf[1024],ipwdbuf[1024],ipwd2buf[1024],dbbuf[1024],userbuf[1024];

        PGconn *con;
        PGresult *res;

        while(1){
                opt=getopt_long(argc,argv,optstring,longopts,0);

                if(opt == -1)break;
                switch(opt){
                        case 'h':
                                printf(HELP,selfname);
                                exit(0);
                                break;
                        case 'v': verbose=1;break;
                        case 'V':
                                printf("%s (%s) version %s\n",PACKAGE,selfname,VERSION);
                                exit(0);
                                break;
                        case 's':server=optarg;break;
                        case 'p':port=optarg;break;
                        case 'd':dbname=optarg;break;
                        case 'u':user=optarg;break;
                        case 'w':
                                if(optarg==0)askpasswd=1;
                                else passwd=optarg;
                                break;
                        case 'o':options=optarg;break;
                        case 'i':
                                doinitfile=1;
                                if(optarg)initfile=optarg;
                                break;
                        case 'U':inituser=optarg;break;
                        case 'P':initpasswd=optarg;break;
                        case 'a':readacc=optarg;break;
                        case 'A':writeacc=optarg;break;
                        case 'S':initsocket=optarg;break;
                        default:
                                fprintf(stderr,"%s: Error, unknown option %c.\n",selfname,opt);
                                exit(1);
                                break;
                }
        }
        if(optind < argc){
                fprintf(stderr,"%s: Error, no non-option arguments allowed.\n",selfname);
                exit(1);
        }

        if(!dbname){
                printf("DB Name: ");
                scanf("%s",dbbuf);
                dbname=dbbuf;
        }
        if(!user){
                printf("DB User: ");
                scanf("%s",userbuf);
                user=userbuf;
        }
        if(askpasswd){
                printf("DB Password: ");
                setecho(0);
                scanf("%s",passwd);
                setecho(1);
        }
        if(!initpasswd){
                setecho(0);
                initpwdagain:/*Dijkstra would hate me ;-)*/
                printf("Initial Password: ");
                scanf("%s",ipwdbuf);
                printf("Repeat Password: ");
                scanf("%s",ipwd2buf);
                if(strcmp(ipwdbuf,ipwd2buf)){
                        printf(" Password mismatch, please retry.\n");
                        goto initpwdagain;
                }
                setecho(1);
        }

        if(verbose)
                printf("Creating DB\n");

        /*misusing ipwd2buf a little bit...*/
        sprintf(ipwd2buf,"createdb -h %s -p %s -U %s %s %s %s",
                server,port,user,passwd?"-W":"",passwd?passwd:"",dbname);
        if(opt=system(ipwd2buf)){
                fprintf(stderr,"Error creating Database: %s\n",
                        opt==-1?"Unable to call createdb.":"Bad return code from createdb.");
                exit(1);
        }

        if(verbose)
                printf("Connecting to DB\n");

        con=PQsetdbLogin(server,port,options,0,dbname,user,passwd);
        if(PQstatus(con)==CONNECTION_BAD){
                fprintf(stderr,"Connection Error: %s\n",PQerrorMessage(con));
                exitdb(con);
        }

        if(verbose)
                printf("Creating tables\n");
        
        res=PQexec(con,SQL_TRANS_BEGIN);
        if(!res || PQresultStatus(res) != PGRES_COMMAND_OK){
                fprintf(stderr,"Unable to create transaction: %s\n",PQerrorMessage(con));
                if(res)PQclear(res);
                exitdb(con);
        }
        PQclear(res);

        res=PQexec(con,SQL_INITDB);
        if(!res || PQresultStatus(res) != PGRES_COMMAND_OK){
                fprintf(stderr,"Unable to create tables: %s\n", PQerrorMessage(con));
                if(res)PQclear(res);
                PQclear(PQexec(con,SQL_TRANS_ROLLBACK));
                exitdb(con);
        }
        PQclear(res);

        res=PQexec(con,SQL_TRANS_COMMIT);
        if(!res || PQresultStatus(res) != PGRES_COMMAND_OK){
                fprintf(stderr,"Unable to commit transaction: %s\n", PQerrorMessage(con));
                if(res)PQclear(res);
                exitdb(con);
        }
        PQclear(res);

        if(verbose)
                printf("Creating initial user\n");

        sprintf(ipwd2buf,"createuser -h %s -p %s -U %s %s %s %s",
                server,port,user,passwd?"-W":"",passwd?passwd:"",inituser);
        if(opt=system(ipwd2buf)){
                fprintf(stderr,"Warning creating User: %s\n",
                        opt==-1?"Unable to call createuser.":"Bad return code from createuser.");
        }
        
/*        res=PQexec(con,query(SQL_GRANT,GRANT_ALL,inituser));
        if(!res || PQresultStatus(res) != PGRES_COMMAND_OK){
                fprintf(stderr,"Unable to grant privileges to initial user %s: %s\n",
                        inituser,PQerrorMessage(con));
                if(res)PQclear(res);
                exitdb(con);
        }*/

        
}