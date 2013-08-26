/* Copyright (c) 1993 Association of Universities for Research 
 * in Astronomy. All rights reserved. Produced under National   
 * Aeronautics and Space Administration Contract No. NAS5-26555.
 */
/* qwrite.c	Write binary data
 *
 * Programmer: R. White		Date: 11 March 1991
 */
#include <stdio.h>
#include <stdlib.h>

extern void writeint();
extern void qwrite();
extern int  mywrite();

extern void
writeint(outfile,a)
FILE *outfile;
int a;
{
int i;
unsigned char b[4];

	/* Write integer A one byte at a time to outfile.
	 *
	 * This is portable from Vax to Sun since it eliminates the
	 * need for byte-swapping.
	 */
	for (i=3; i>=0; i--) {
		b[i] = a & 0x000000ff;
		a >>= 8;
	}
	for (i=0; i<4; i++) qwrite(outfile,&b[i],1);
}

extern void
qwrite(outfile,a,n)
FILE *outfile;
char *a;
int n;
{
	if(mywrite(outfile, a, n) != n) {
		perror("qwrite");
		exit(-1);
	}
}

extern int
mywrite(file, buffer, n)
FILE *file;
char buffer[];
int n;
{
    /*
     * read n bytes from file into buffer
     * returns number of bytes read (=n) if successful, <=0 if not
     */
    return ( fwrite(buffer, 1, n, file) );
}
