/***************************************************************************
                          query.h  -  description
                             -------------------
    begin                : Sun Oct 6 2002
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

#ifndef BAGD_QUERY_H
#define BAGD_QUERY_H

/** format a query

 format-string:

 %s - string
 %b - bytea, this is the only one requiring two parameters: void*bin,size_t len
 %i - integer
 %h - short int
 %l - long int
 %L - long long int
 %f - float/double
 %o - OID
 %% - single % sign

 call query(0) to just free the buffer

*/
const char* query(const char*format,...);


#define AUTH_INTERN     1
#define AUTH_PAM        2
#define AUTH_SSL        4
#define AUTH_ANON       0x4000
#define AUTH_LOCKED     0x8000


#define ACL_



#endif /*BAGD_QUERY_H*/
