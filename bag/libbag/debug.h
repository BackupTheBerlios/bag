/***************************************************************************
                          debug.h  -  debugger functions
                             -------------------
    begin                : Sun Sep 14 2003
    copyright            : (C) 2003 by Konrad Rosenbaum
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

#include "log.h"

#define debug(arg, ...) log(LOG_DEBUG,arg, ## __VA_ARGS__)

#define ASSERT(b) if(!(b)){log(LOG_ERR,"Fatal error, ASSERT failed in %s line %i.",__FILE__,__LINE__);exit(126);}
