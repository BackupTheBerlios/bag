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

static const char* getlevel(int level)
{
        switch(level)
        {
                case LOG_ERR:return "Error";break;
                case LOG_WARNING:return "Warning";break;
                case LOG_INFO:return "Info";break;
                case LOG_DEBUG:return "Debug";break;
                default:return "NoLevel";break;
        }
}

void log(int level,const char*format,...)
{
        va_list arg;
        va_start(arg,format);
        char buf[1024];
        vsnprintf(buf,sizeof(buf),format,arg);
        buf[sizeof(buf)-1]=0;
        if(tosyslog){
                syslog(level,"%s",buf);
        }else{
                fprintf(stderr,"%s: %s\n",getlevel(level),buf);
        }
        va_end(arg);
}
