/***************************************************************************
                          dynbuf.h  -  dynamic buffer
                             -------------------
    begin                : 2003-09-14
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

struct dynbuf_s {
        int len,plen;
        void *buffer;
};

struct dynbuf_s* dynbuf_new();
int dynbuf_addstr(struct dynbuf_s*,const char*);
void dynbuf_free(struct dynbuf_s*);

