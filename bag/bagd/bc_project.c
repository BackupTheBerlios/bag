/***************************************************************************
                          bc_project.c  -  description
                             -------------------
    begin                : Thu Mar 6 2003
    copyright            : (C) 2003 by Konrad Rosenbaum
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

#include "bc_project.h"
#include "bagchild.h"
#include "bc_user.h"
#include "bc_auth.h"
#include "query.h"
#include "defines.h"
#include "sql.h"
#include "bagd.h"
#include "log.h"

#include "../libbag/dynbuf.h"

#include <stdlib.h>


void projects_handler(int argc,char**argv,int bloblen,void*blob);
void createproject_handler(int argc,char**argv,int bloblen,void*blob);
void projectbranches_handler(int argc,char**argv,int bloblen,void*blob);
void projectacl_handler(int argc,char**argv,int bloblen,void*blob);
void projectbranchacl_handler(int argc,char**argv,int bloblen,void*blob);
void projectcreatebranch_handler(int argc,char**argv,int bloblen,void*blob);

