/***************************************************************************
                          defines.h  -  standard values
                             -------------------
    begin                : Sun Dec 29 2002
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


#ifndef BAGD_DEFINES_H
#define BAGD_DEFINES_H

#define E_OK "+0 OK\n"

#define E_STORAGE "-0 internal storage error, please contact the server admin.\n"
#define E_ALLOCATION "-0 not enough memory to execute\n"
#define E_SYNTAX "-0 Syntactical error.\n"
#define E_PERM "-0 Insufficient Karma.\n"
#define E_DATABASE "-0 Internal Database Error, please contact the server admin.\n"

#define E_UNIMPL "-0 Yet not implemented. Sorry. :-(\n"

/*specific for bagchild.c:*/
#define E_LINETOOLONG "-0 line too long\n"
#define E_ILLEGALSEQUENCE "-0 illegal backlash sequence\n"
#define E_BLOBLEN "-0 illegal value for blob length\n"
#define E_NOBLOB "-0 this function expects no blob\n"
#define E_READBLOB "-0 unable to read complete blob\n"
#define E_NOCOMMAND "-0 no such command or command not allowed in current mode\n"
#define E_PROTOCOL "-0 Protocol error.\n"

/*other specifics*/
#define E_NOOPT "-0 No such Option.\n"



#define AUTH_INTERN     1
#define AUTH_PAM        2
#define AUTH_SSL        4
#define AUTH_ANON       0x4000
#define AUTH_LOCKED     0x8000

/*
#define ACL_GLOBAL 0
#define ACL_PROJECT 1
#define ACL_BRANCH 2
*/

/*global ACLs:*/
#define ACL_ADMIN 'A'
#define ACL_READ 'r'
#define ACL_WRITE 'w'
#define ACL_CONFIG 'c'
#define ACL_ADMPROJECT 'p'
#define ACL_ADMUSER 'u'

/*Project ACLs:*/
#define ACL_USRBRANCH 'b'
#define ACL_BRANCH 'B'

/*Branch ACLs:*/
#define ACL_FULL 'W'
#define ACL_DELETE 'd'
#define ACL_ADD 'a'
#define ACL_TAG 't'
#define ACL_MERGE 'm'


#endif /* BAGD_DEFINES_H*/
