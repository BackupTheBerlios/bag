/***************************************************************************
                          bagssl.c  -  description
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

#include "../config.h"
#include "bagssl.h"
#include "log.h"

#include <gnutls/gnutls.h>
#include <gnutls/extra.h>

struct s_sockethandler *newsslhandler(int fd)
{
        close(fd);
        exit(1);
}
