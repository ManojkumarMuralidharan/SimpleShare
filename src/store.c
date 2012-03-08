/*
 * =====================================================================================
 *
 *       Filename:  split.c
 *
 *    Description:  ShareDAV - Store
 *
 *        Version:  1.0
 *        Created:  01/24/2012 08:59:02 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Stephan Laukien (Ditha), software@laukien.com
 *        Company:  
 *
 * =====================================================================================
 */

#include "common.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void store(char *src);

int main(int argc, char *argv[]) {
	showCopyright("Store");
	initConfig();

	/* check arguments */
	if (argc != 2) {
		printf ( "ERROR: Invalid arguments.\n" );
		printf ( "\t%s FILE-TO-SPLIT\n", argv[0] );
		exit(EXIT_FAILURE);
	}

	char *filename = argv[1];

	if (!fileExists(filename)) {
		printf ( "ERROR: File (%s) doesn't exists.\n", filename );
		exit(EXIT_FAILURE);
	}

	store(filename);

	releaseConfig();
	return(EXIT_SUCCESS);
}

void store(char *src) {
	char *dirfile;
	char *dst_store = getConfigStore();
	char *dst_key = getConfigKey();
	int dirsize = strlen(dst_store);
	int filesize = dirsize + 1 + 4 + 1 + KEY_SIZE;

    dirfile = malloc(filesize + 1);             /* dir-size/4/md5\0 */
	if (dirfile == NULL) {
		printf ( "ERROR: Unable to get memory.\n" );
		exit(EXIT_FAILURE);
	}
    memcpy(dirfile, dst_store, dirsize);        /* copy directory into dirfile-buffer */
	dirfile[dirsize] = '/';                     /* insert a separator */
    dirfile[filesize] = '\0';                   /* end with \0 */

    char buffer[BLOCK_SIZE+1];                  /* buffer +1 for final \0 */

	/* database */
	char *srcname = getFilename(src);
	char *keyname;
    keyname = (char*)malloc(strlen(dst_key)+1+strlen(srcname)+1+14+1+3+1); /* dst_key/src.timestamp.key+\0 */
	if (keyname == NULL) {
		printf ( "ERROR: Unable to get memory.\n" );
		exit(EXIT_FAILURE);
	}
	sprintf(keyname, "%s/%s.%s.key", dst_key, srcname, getTimestampAsString());

	/* key-file */
	FILE *keyfile;
	keyfile = fopen(keyname, "a");

	/* input-file */
	FILE *in;
	in = fopen(src, "rb");
	if (in == NULL ) {
		printf ( "ERROR: Unable to write file (%s).\n", src );
		exit(EXIT_FAILURE);
	}

	unsigned long long size_store = 0;
	unsigned long long size_save = 0;
	size_t size;
	unsigned int count = 0;
	unsigned int exist = 0;
	char signature[KEY_SIZE+1];
    memset(buffer, '\0', BLOCK_SIZE+1);         /* initial clearing */
	while ((size = fread(buffer, 1, BLOCK_SIZE, in))) {
        ++count;                                /* line-counter */
		size_store += size;
		/* build destination-filename */
		strcpy(signature, calculateSignature(buffer, size));
		memcpy(dirfile+dirsize+1, signature, 4);
        dirfile[dirsize+4+1] = '\0';            /* cut string to parent-directory */
        mkdir(dirfile, 0755);                   /* make parent-directory */
        dirfile[dirsize+4+1] = '/';             /* second separator */
		memcpy(dirfile+dirsize+1+4+1, signature, KEY_SIZE);

		printf ( "FILE-%d: %s", count, signature );
		if (fileExists(dirfile)) {
			++exist;
			size_save += size;
			printf ( " (EXISTS)" );
		} else {
			/* write data-file */
			FILE *out;
			out = fopen(dirfile, "wb");
			fwrite(buffer, 1, size, out);
			fclose(out);
		}
		printf ( "\n" );

		/*  write key-file */
		fprintf(keyfile, "%s\n", signature);

		/* clean */
        memset(buffer, '\0', BLOCK_SIZE+1);     /* clear 'buffer' - for strlen */
	}

	printf ( "KEY: %s\n", keyname);
	printf ( "FILE-SIZE: %lld\n", size_store);
	printf ( "FILE-SAVE: %lld\n", size_save);
	if (size_save == 0) {
		printf ( "FILE-RATIO: 0%%\n\n" );
	} else {
		printf ( "FILE-RATIO: %lld%%\n", size_save*100/size_store);
	}

	/* free resources */
	free(srcname);
	free(keyname);
	free(dirfile);

	fclose(in);
	fclose(keyfile);
}
