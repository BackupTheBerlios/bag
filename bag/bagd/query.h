/***************************************************************************
                          query.h  -  description
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

#ifndef BAGD_QUERY_H
#define BAGD_QUERY_H

#include "../config.h"

/*debug-level*/
extern int querydebug;


#ifdef HAVE_POSTGRES
/*currently only postgres is supported:*/
#include <libpq-fe.h>
typedef PGresult dbResult;
typedef PGconn dbConn;
#else
# error unable to compile DB support, no valid DB-System available
#endif

/** format a query

 format-string:

 %s - string
 %b - bytea, this is the only one requiring two parameters: void*bin,size_t len
 %i - integer
 %h - short int
 %l - long int
 %L - long long int
 %f - float/double
 %o - OID
 %% - single % sign

 call query(0) to just free the buffer

*/
dbResult* query(dbConn*con,const char*format,...);
dbConn*dbConnect(const char*constring);
int dbUpdateOK(dbResult*);
int dbSelectOK(dbResult*);
void dbClose(dbConn*con);
void dbFreeResult(dbResult*);
void dbFree(dbResult*);
int dbNumRows(dbResult*);
char*dbGetString(dbResult*r,int row,int col);
char*dbGetStringByname(dbResult*r,int row,const char* col);
int dbGetFieldIndex(dbResult*r,const char*colname);
int dbIsNull(dbResult*r,int row,int col);
int dbIsNullByname(dbResult*r,int row,const char* col);
int dbGetInt(dbResult*r,int row,int col);
int dbGetIntByname(dbResult*r,int row,const char* col);

int dbAffectedRows(dbResult*);


#endif /*BAGD_QUERY_H*/
