/***************************************************************************
                          bagchild.h  -  description
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

#ifndef BAGCHILD_H
#define BAGCHILD_H

#include <libpq-fe.h>

struct s_sockethandler;

typedef int (*sockhandler_t)(struct s_sockethandler*,void*buf,int len);
typedef void (*sockcloser_t)(struct s_sockethandler*);
typedef int (*sockauth_t)(struct s_sockethandler*,char*cert);

struct s_sockethandler{
        sockhandler_t sockreader,sockwriter;
        sockcloser_t sockcloser;
        sockauth_t sockauth;
        int fd;
        void*data;
};

typedef void (*linehandler_t)(int argc,char**argv,int bloblen,void*blob);

struct s_linehandler{
        const char*command;
        int hasblob;
        linehandler_t linehandler;
};

void bagchild(struct s_sockethandler*hdl,const char*dbstring);

/*needs to be changed if we convert to threads instead processes
 probably a macro requesting thread specific values*/
extern PGconn *bc_con;
extern struct s_sockethandler*bc_hdl;

/*returns the number of the command executed or -1 in error*/
int processline(struct s_linehandler*);

#define E_ALLOCATION "-0 not enough memory to execute\n"


/*close/destroy child process (does not return)*/

void closechild();


#endif
