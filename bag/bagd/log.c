/***************************************************************************
                          log.c  -  description
                             -------------------
    begin                : Fri Oct 4 2002
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

#include "log.h"
#include <stdarg.h>
#include <stdio.h>

int tosyslog=0;

void log(int level,const char*format,...)
{
        va_list arg;
        va_start(arg,format);
        if(tosyslog){
                char buf[1024];
                vsnprintf(buf,sizeof(buf),format,arg);
                syslog(level,"%s",buf);
        }else vfprintf(stderr,format,arg);
        va_end(arg);
}
