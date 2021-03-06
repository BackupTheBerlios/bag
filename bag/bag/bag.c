/***************************************************************************
                          bag.c  -  description
                             -------------------
    begin                : Tue Aug 20 2002
    copyright            : (C) 2002 by konrad
    email                : konrad@zaphod
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>



int main(int argc,char**argv)
{
        char *ln;
        for(;;){
                ln=readline("bag> ");
                if(ln){
                        if(*ln){
                                add_history(ln);
                                /*parse line*/
                                if(!strcmp("exit",ln))return 0;
                        }
                        free(ln);
                }else return 0;
        }
}

