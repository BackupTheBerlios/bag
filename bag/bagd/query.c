/***************************************************************************
                          query.c  -  description
                             -------------------
    begin                : Sun Oct 6 2002
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

#include "query.h"
#include "log.h"

/*debug-level*/
int querydebug=0;


#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

dbConn*dbConnect(const char*constring)
{
        PGconn *con;
        con=PQconnectdb(constring);
        if(PQstatus(con)==CONNECTION_BAD){
                log(LOG_ERR,"Connection Error: %s",PQerrorMessage(con));
                PQfinish(con);
                return 0;
        }else return con;
}

void dbClose(dbConn*con)
{
        PQfinish(con);
}

void dbFreeResult(dbResult*res){dbFree(res);}
void dbFree(dbResult*res)
{
        PQclear(res);
}

int dbUpdateOK(dbResult*res)
{
        return res && PQresultStatus(res)==PGRES_COMMAND_OK;
}

int dbSelectOK(dbResult*res)
{
        return res && PQresultStatus(res)==PGRES_TUPLES_OK;
}

int dbNumRows(dbResult*res)
{
        return PQntuples(res);
}

dbResult* query(dbConn *con,const char*format,...)
{
        static char*ret=0;
        char**parms;
        int i,j,k,pstat,cnt,size,flen;
        va_list va;
        PGresult *res;
        /*exception: if format = NULL then delete old query only*/
        if(ret){
                free(ret);
                ret=0;
        }
        if(format==0)return NULL;
        /*stage 1 - count parameters*/
        flen=strlen(format);
        for(cnt=pstat=i=0;i<flen;i++)
                if(format[i]=='%'){
                        if(pstat){cnt--;pstat=0;}/*for %% count down again*/
                        else cnt++; /*for every % count up*/
                }else pstat=0;/*reset %-state for any  non-%*/
                
        /*stage 2 - estimate memory*/
        parms=malloc(sizeof(char*)*cnt);
        for(i=0;i<cnt;i++)parms[i]=0;
        size=flen;
        va_start(va,format);
        /*i=pos in format, j=pos in parms*/
        for(pstat=i=j=0;i<flen;i++)
                if(pstat){
                        char*s;
                        long long l;
                        size_t l2;
                        double d;
                        pstat=0;
                        switch(format[i]){
                                case '%':break;
                                case 's':
                                        s=va_arg(va,char*);
                                        parms[j]=malloc(strlen(s)*2+1);
                                        PQescapeString(parms[j],s,strlen(s));
                                        size+=strlen(parms[j]);
                                        j++;
                                        break;
                                case 'b':
                                        s=va_arg(va,void*);
                                        l=va_arg(va,size_t);
                                        /*PQescapeBytea malloc's enough mem, it must be free'd by us*/
                                        parms[j]=PQescapeBytea(s,l,&l2);
                                        size+=l2;
                                        j++;
                                        break;
                                case 'h':/*GNU C: short is transformed to int if passed through ...*/
                                case 'i':
                                        l=va_arg(va,int);
                                        parms[j]=malloc(22);/*the highest long long is 20 chars, if int happens to be 64 bit*/
                                        sprintf(parms[j],"%i",(int)l);
                                        size+=strlen(parms[j]);
                                        j++;
                                        break;
                                case 'l':
                                        l=va_arg(va,long);
                                        parms[j]=malloc(22);/*the highest long long is 20 chars, if int happens to be 64 bit*/
                                        sprintf(parms[j],"%li",(long)l);
                                        size+=strlen(parms[j]);
                                        j++;
                                        break;
                                case 'L':
                                        l=va_arg(va,long long);
                                        parms[j]=malloc(22);/*the highest long long is 20 chars, if int happens to be 64 bit*/
                                        sprintf(parms[j],"%lli",(long long)l);
                                        size+=strlen(parms[j]);
                                        j++;
                                        break;
                                case 'f':/*float and double are passed as double in ...*/
                                        d=va_arg(va,double);
                                        parms[j]=malloc(32);/*should be more than enough*/
                                        sprintf(parms[j],"%e",d);
                                        size+=strlen(parms[j]);
                                        j++;
                                        break;
                                case 'o':
                                        l=va_arg(va,Oid);/*Oid is a kind of integer*/
                                        parms[j]=malloc(22);/*the highest long long is 20 chars, if int happens to be 64 bit*/
                                        sprintf(parms[j],"%lli",(long long)l);
                                        size+=strlen(parms[j]);
                                        j++;
                                        break;
                                default:/*error, unknown type*/
                                        log(LOG_WARNING,"Unknown Query-Parameter type: %c",format[i]);
                                        for(i=0;i<cnt;i++)
                                                if(parms[i])free(parms[i]);
                                        free(parms);
                                        return 0;
                                
                        }
                }else if(format[i]=='%')pstat=1;
        
        
        /*stage 3 - fill query*/
        ret=malloc(size+1);
        /*i=pos in format, j=pos in parms, k=pos in ret*/
        for(i=j=pstat=k=0;i<flen;i++)
                if(pstat){
                        long l;
                        switch(format[i]){
                                case '%':ret[k++]='%';break;
                                default:
                                        if(querydebug>1)
                                                log(LOG_DEBUG,"Query-Parm: %s",parms[j]);
                                        l=strlen(parms[j]);
                                        memcpy(ret+k,parms[j],l);
                                        k+=l;
                                        j++;
                                        break;
                        }
                        pstat=0;
                }else{
                        if(format[i]=='%')pstat=1;
                        else ret[k++]=format[i];
                }
        ret[k]=0;

        /*return*/
        for(i=0;i<cnt;i++)
                if(parms[i])free(parms[i]);
        free(parms);

        if(querydebug)
                log(LOG_DEBUG,"Query (%i): %s",cnt,ret);

        res=PQexec(con,ret);
        if(!res || (PQresultStatus(res)!=PGRES_TUPLES_OK && PQresultStatus(res)!=PGRES_COMMAND_OK)){
                log(LOG_WARNING,"DB-Error: %s",PQerrorMessage(con));
                if(res)PQclear(res);
                return 0;
        }
        return res;
}


char*dbGetString(dbResult*r,int row,int col)
{
        return PQgetvalue(r,row,col);
}

char*dbGetStringByname(dbResult*r,int row,const char* col)
{
        return PQgetvalue(r,row,PQfnumber(r,col));
}

int dbGetFieldIndex(dbResult*r,const char*colname)
{
        return PQfnumber(r,colname);
}

int dbIsNull(dbResult*r,int row,int col)
{
        return PQgetisnull(r,row,col);
}

int dbIsNullByname(dbResult*r,int row,const char* col)
{
        return PQgetisnull(r,row,PQfnumber(r,col));
}

int dbGetInt(dbResult*r,int row,int col)
{
        return atoi(PQgetvalue(r,row,col));
}

int dbGetIntByname(dbResult*r,int row,const char* col)
{
        return atoi(PQgetvalue(r,row,PQfnumber(r,col)));
}

int dbAffectedRows(dbResult*r)
{
        if(!dbUpdateOK(r))return -1;
        else return atoi(PQcmdTuples(r));
}
