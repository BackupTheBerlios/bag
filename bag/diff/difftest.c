/***************************************************************************
                          difftext.c  -  description
                             -------------------
    begin                : Sat Sep 7 2002
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
#include "diff.h"

main()
{
        struct file_t *src,*tgt;
        struct patch_t*pt;
        src=newfile("source.txt");
        tgt=newfile("target.txt");
        pt=diff(src,tgt);
        printpatch(pt);
        freefile(src);
        freefile(tgt);
        freepatch(pt);
}
