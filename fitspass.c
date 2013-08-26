/* Copyright (c) 1993 Association of Universities for Research 
 * in Astronomy. All rights reserved. Produced under National   
 * Aeronautics and Space Administration Contract No. NAS5-26555.
 */
/* fitspass.c	Read FITS header from infile and pass it to outfile,
 *				Adds blank lines to make header a multiple of 36 lines.
 *				This may exit with error status if some problem is found.
 *
 * Programmer: R. White		Date: 16 April 1992
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void
fitspass(infile,passthru,outfile)
FILE *infile;			/* input file */
int  passthru;			/* non-zero -> copy header to outfile */
FILE *outfile;			/* output file */
{
char line[81];
int i, j;

	/*
	 * Note that the SIMPLE line has already been stripped off and written
	 * to outfile for FITS files, so we start at i=1
	 */
	for (i = 1; ; i++) {
		if (fgets(line,81,infile)==NULL) {
			fprintf(stderr,"FITS header has no END statement\n");
			exit(-1);
		}
		if (strlen(line)!=80) {
			fprintf(stderr, "Error in FITS header: found embedded newline\n");
			exit(-1);
		}

		if (passthru) fputs(line,outfile); 

		if (strncmp(line,"END ",4) == 0) break;
	}
	/*
	 * write blank lines to make a multiple of 36 lines in header
	 * number of lines written so far is i+1
	 */
	if (passthru) {
		for (j=0; j<80; j++) line[j]=' ';
		line[80]='\0';
		for (i = 35 - (i % 36); i>0; i--) fputs(line,outfile);
	}
}
