/***************************************************************************
                          diff.c  -  description
                             -------------------
    begin                : Sun Sep 1 2002
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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>


/*To differ between new lines and inserted lines the algorithm searches forward
 *in the file, if it finds something the text between was inserted. The problem is
 *that a) inserted lines may be identical to other lines still there, b) inserted
 *lines may be identical to other deleted lines, which would create a match anyway.
 *These values will restrict the search to the smaller value of either MAXSEARCH lines
 *or file length (in lines) divided by DIVSEARCH. The assumption is that a programmer seldom
 *adds more than a certain amount of lines or changes more than a certain fraction
 *of the file.
 */
#define MAXSEARCH 128
#define DIVSEARCH 3




struct file_t* newfile(const char*fl)
{
        int fd,len;
        char*cont;
        fd=open(fl,O_RDONLY);
        if(fd<0){
                fprintf(stderr,"Error reading file %s: %s\n",fl,strerror(errno));
                exit(1);
        }
        len=lseek(fd,0,SEEK_END);
        lseek(fd,0,SEEK_SET);
        cont=malloc(len+1);
        cont[len]=0;
        read(fd,cont,len);
        close(fd);
        return newbuffer(cont,len,0);
}



/*buf: the buffer
 *len: the length of its content
 *copy: copy it? if this is 0, the buffer must be 0-terminated (cont[len]==0)
 */
struct file_t* newbuffer(char*buf,int len,int copy)
{
        int i,j;
        struct file_t*f;
        char*cont;
        f=malloc(sizeof(struct file_t));
        f->lines=1;
        if(copy){
                cont=malloc(len+1);
                cont[len]=0;
                memcpy(cont,buf,len);
        }else{
                cont=buf;
        }
        for(i=0;i<len;i++)if(cont[i]=='\n')f->lines++;
        f->line=malloc(sizeof(char*)*f->lines);
        f->line[0]=cont;
        j=1;
        for(i=0;i<len;i++)if(cont[i]=='\n'||cont[i]=='\r'){
                int x=0;
                if((cont[i]=='\n'&&cont[i+1]=='\r')||(cont[i]=='\r'&&cont[i+1]=='\n'))x=1;
                cont[i]=0;
                if(x)i++;
                f->line[j++]=cont+i+1;
        }

        return f;
}

void freefile(struct file_t*f)
{
        if(f->line)free(f->line[0]);
        free(f);
}

struct patch_t* diff(struct file_t*src,struct file_t*tgt)
{
        int i,j,maxsearch;
        struct patch_t*pt;

        pt=malloc(sizeof(struct patch_t));
        pt->line=malloc((src->lines+tgt->lines)*sizeof(char*));
        pt->status=malloc(tgt->lines);
        pt->iscopy=0;
        pt->lines=0;
        for(i=0;i<pt->lines;i++){
                pt->status[i]=0;
                pt->line[i]=NULL;
        }

        maxsearch=src->lines/DIVSEARCH;
        if(maxsearch>MAXSEARCH)maxsearch=MAXSEARCH;
        for(i=j=0;i<tgt->lines||j<src->lines;i++){
                if(j>=src->lines){/*it's new line at the end*/
                        pt->status[pt->lines]=2;
                        pt->line[pt->lines]=tgt->line[i];
                        pt->lines++;
                        continue;
                }
                if(i>=tgt->lines){/*it has been deleted*/
                        pt->status[pt->lines]=1;
                        pt->line[pt->lines]=src->line[j];
                        pt->lines++;
                        j++;
                        continue;
                }
                if(!strcmp(src->line[j],tgt->line[i])){/*a identical line*/
                        pt->status[pt->lines]=3;
                        pt->line[pt->lines]=tgt->line[i];
                        pt->lines++;
                        j++;
                        continue;
                }else{
                        int k,m,l;
                        m=j+maxsearch;
                        if(m>src->lines)m=src->lines;
                        for(k=j;k<m;k++){/*search the line*/
                                if(!strcmp(src->line[k],tgt->line[i])){
                                        break;
                                }
                        }
                        if(k==m){/*the line wasn't found, hence it's inserted*/
                                pt->status[pt->lines]=2;
                                pt->line[pt->lines]=tgt->line[i];
                                pt->lines++;
                        }else{/*line was found, hence status=0, dels>0*/
                                for(l=j;l<k;l++){
                                        pt->status[pt->lines]=1;
                                        pt->line[pt->lines]=src->line[l];
                                        pt->lines++;
                                }
                                pt->status[pt->lines]=3;
                                pt->line[pt->lines]=tgt->line[i];
                                pt->lines++;
                                j=k+1;
                        }
                }
        }

        return pt;
}

struct patch_t*patchcopy(struct patch_t*p)
{
        int i;
        char*l;
        if(p->iscopy)return p;
        for(i=0;i<p->lines;i++){
                l=malloc(strlen(p->line[i])+1);
                strcpy(l,p->line[i]);
                p->line[i]=l;
        }
        p->iscopy=1;
        return p;
}

void freepatch(struct patch_t*p)
{
        if(p->iscopy){
                int i;
                for(i=0;i<p->lines;i++)
                        if(p->line[i])free(p->line[i]);
        }
        if(p->status)free(p->status);
        free(p);
}

void printpatch(struct patch_t*p)
{
        int i;
        char s;
        for(i=0;i<p->lines;i++){
                switch(p->status[i]){
                        case 1:s='-';break;
                        case 2:s='+';break;
                        case 3:s='=';break;
                        default: s='!';break;
                }
                printf("%c%s\n",s,p->line[i]);
        }
}


