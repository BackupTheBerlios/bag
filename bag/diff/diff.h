/***************************************************************************
                          diff.h  -  description
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

#ifndef BAG_DIFF_H
#define BAG_DIFF_H

#ifndef CPLUSPLUS
#define bool char
#endif


/*lines: number of lines
 *line: array of lines (the last one is empty if a trailing newline is encountered)
 */
struct file_t {
        int lines;
        char**line;
};


/*lines: amount of lines in this patch (src->lines + tgt->lines at maximum)
 *line: content of the line
 *status: 1=in source only (-), 2=in target only (+), 3=in both of them (=)
 *iscopy: if 0 line[0] points to the complete character array, else each line
 *  was allocated separately (yet always 0)
 */
struct patch_t {
        int lines;
        char **line;
        char *status;
        bool iscopy;
};

/*FUNCTIONS
 *newfile: allocate and load a file
 *newbuffer: allocate a file, fill from buffer
 *freefile: free the file structure
 *diff: create a patch between two files
 *patchcopy: copy all strings in a patch structure
 *freepatch: free a patch structure
 *printpatch: print the patch to stdout
 */


struct file_t* newbuffer(char*,int,int);
struct file_t* newfile(const char*fl);
struct file_t* newbuffer(char*buf,int len,int copy);
void freefile(struct file_t*f);
struct patch_t* diff(struct file_t*src,struct file_t*tgt);
struct patch_t*patchcopy(struct patch_t*p);
void freepatch(struct patch_t*p)
void printpatch(struct patch_t*p)


#endif /*BAG_DIFF_H*/