/***************************************************************************
                          misc.c  -  description
                             -------------------
    begin                : Thu Oct 24 2002
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

#include <libpq-fe.h>
#include <sys/types.h>
#include "misc.h"
 
void md5_hex_buffer (const char *buffer, size_t len, char*resblock)
{
        unsigned char buf[16];
        char hex[]="0123456789abcdef";
        int i;
        md5_buffer(buffer,len,buf);
        for(i=0;i<16;i++){
                resblock[i*2]=hex[(buf[i]>>4)&0xf];
                resblock[i*2+1]=hex[buf[i]&0xf];
        }
        resblock[32]=0;
}


