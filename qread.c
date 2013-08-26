/* Copyright (c) 1993 Association of Universities for Research 
 * in Astronomy. All rights reserved. Produced under National   
 * Aeronautics and Space Administration Contract No. NAS5-26555.
 */
/* qread.c	Read binary data
 *
 * Programmer: R. White		Date: 11 March 1991
 */
#include <stdio.h>
#include <stdlib.h>

extern int  readint();
extern void qread();
extern int  myread();

extern int
readint(infile)
FILE *infile;
{
int a,i;
unsigned char b[4];

	/* Read integer A one byte at a time from infile.
	 *
	 * This is portable from Vax to Sun since it eliminates the
	 * need for byte-swapping.
	 */
	for (i=0; i<4; i++) qread(infile,(char *) &b[i],1);
	a = b[0];
	for (i=1; i<4; i++) a = (a<<8) + b[i];
	return(a);
}

extern void
qread(infile,a,n)
FILE *infile;
char *a;
int n;
{
	if(myread(infile, a, n) != n) {
		perror("qread");
		exit(-1);
	}
}

extern int
myread(file, buffer, n)
FILE *file;
char buffer[];
int n;
{
    /*
     * read n bytes from file into buffer
     * returns number of bytes read (=n) if successful, <=0 if not
     *
     * this version is for VMS C: each read may return
     * fewer than n bytes, so multiple reads may be needed
     * to fill the buffer.
     *
     * I think this is unnecessary for Sun Unix, but it won't hurt
     * either, so I'll leave it in.
     */
    int nread, total;

    /* keep reading until we've read n bytes */
    total = 0;
    while ( (nread = fread(&buffer[total], 1, n-total, file)) > 0) {
	total += nread;
	if (total==n) return(total);
    }
    /* end-of-file or error occurred if we got to here */
    return(nread);
}
