/***************************************************************************
                          dynbuf.c  -  dynamic buffer
                             -------------------
    begin                : Sun Sep 14 2003
    copyright            : (C) 2003 by konrad
    email                : konrad@silmor.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dynbuf.h"
#include <stdlib.h>
#include "debug.h"
#include <string.h>

/*buffer delta is 1kB*/
#define DELTA 1024


int dynbuf_ensure(struct dynbuf_s*b,int sz)
{
        int l;
        void *buf;
        l=b->plen;
        while(l<sz)l+=DELTA;
        if(l!=b->plen){
                buf=realloc(b->buffer,l);
                if(!buf)return 1;
                b->buffer=buf;
                b->plen=l;
        }
        return 0;
}

struct dynbuf_s* dynbuf_new()
{
        struct dynbuf_s*r;
        r=malloc(sizeof(struct dynbuf_s));
        ASSERT(r);
        r->buffer=malloc(DELTA);
        ASSERT(r->buffer);
        r->len=0;
        r->plen=DELTA;
        return r;
}

int dynbuf_addstr(struct dynbuf_s*b,const char*s)
{
        int l=strlen(s);
        if(dynbuf_ensure(b,b->len+l+1))return 1;
        memcpy(((char*)(b->buffer))+b->len,s,l);
        b->len+=l;
        ((char*)(b->buffer))[b->len]=0;
        return 0;
}

void dynbuf_free(struct dynbuf_s*b)
{
        free(b->buffer);
        free(b);
}
