/***************************************************************************
              bc_user.h  -  user/config manipulation (subroutine of bc_main)
                             -------------------
    begin                : Sat Dec 28 2002
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

#ifndef BAGD_BC_USER_H
#define BAGD_BC_USER_H

void config_handler(int argc,char**argv,int bloblen,void*blob);
void getusers_handler(int argc,char**argv,int bloblen,void*blob);
void getuseracl_handler(int argc,char**argv,int bloblen,void*blob);
void setuseracl_handler(int argc,char**argv,int bloblen,void*blob);
void createuser_handler(int argc,char**argv,int bloblen,void*blob);
void dropuser_handler(int argc,char**argv,int bloblen,void*blob);
void setpassword_handler(int argc,char**argv,int bloblen,void*blob);
void disableuser_handler(int argc,char**argv,int bloblen,void*blob);
void enableuser_handler(int argc,char**argv,int bloblen,void*blob);

/*see query.h for ACL_* values*/
int checkGlobalACL(char capability);


#endif /*BAGD_BC_USER_H*/
