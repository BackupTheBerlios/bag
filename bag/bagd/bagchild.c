/***************************************************************************
                          bagchild.c  -  description
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

#include "bagchild.h"

void bagchild(struct s_sockethandler*hdl,const char*dbstring)
{
        hdl->sockwriter(hdl,"-Sorry still trying to get this running.\n",41);


        hdl->sockcloser(hdl);
        return; /*return to spawnchild and exit there*/
}
