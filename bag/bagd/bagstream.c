/***************************************************************************
                          bagstream.c  -  description
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

#include "bagstream.h"

static int writestream(struct s_sockethandler*hdl,void*buf,int len)
{
        return write(hdl->fd,buf,len);
}

static int readstream(struct s_sockethandler*hdl,void*buf,int len)
{
        return read(hdl->fd,buf,len);
}

static void closestream(struct s_sockethandler*hdl)
{
        close(hdl->fd);
        free(hdl);
}

struct s_sockethandler *newstreamhandler(int fd)
{
        struct s_sockethandler*s;
        s=malloc(sizeof(struct s_sockethandler));
        s->fd=fd;
        s->sockreader=readstream;
        s->sockwriter=writestream;
        s->sockcloser=closestream;
        s->sockauth=0;
        s->data=0;
        return s;
}
