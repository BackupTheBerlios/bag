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

struct s_sockethandler;

typedef int (*sockhandler_t)(struct s_sockethandler*,void*buf,int len);
typedef void (*sockcloser_t)(struct s_sockethandler*);
typedef int (*sockauth_t)(struct s_sockethandler*);

struct s_sockethandler{
        sockhandler_t sockreader,sockwriter;
        sockcloser_t sockcloser;
        sockauth_t sockauth;
        int fd;
        void*data;
};

void bagchild(struct s_sockethandler*hdl,const char*dbstring);


#endif
