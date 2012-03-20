/*
 * =====================================================================================
 *
 *       Filename:  concat.c
 *
 *    Description:  concatente all parts to one file
 *
 *        Version:  1.0
 *        Created:  01/24/2012 03:08:09 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Stephan Laukien (slaukien), software@laukien.com
 *        Company:  
 *
 * =====================================================================================
 */

#include "common.h"
#include "config.h"
#include "la_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void restore(char *src);

int main(int argc, char *argv[]) {
	showCopyright("Restore");
	initConfig();

	/* check arguments */
	if (argc != 2) {
		printf ( "ERROR: Invalid arguments.\n" );
		printf ( "\t%s DATABASE-FILE\n", argv[0] );
		exit(EXIT_FAILURE);
	}

	char *filename = argv[1];
	char *filename_abs = getAbsoluteKeyname(filename);
	if (!file_exists(filename_abs)) {
		free(filename_abs);
		printf ( "ERROR: File (%s) doesn't exists.\n", filename );
		exit(EXIT_FAILURE);
	}
	free(filename_abs);

	restore(filename);

	releaseConfig();
	return(EXIT_SUCCESS);
}

void restore(char *src) {
	/* prepare filename (store/1234/... */
	char *dir_store = getConfigStore();
	int dir_len = strlen(dir_store) + 1 + 4 + 1 + KEY_SIZE + 1;
	char *file_store;
	file_store = (char*)malloc(dir_len);
	if (file_store == NULL) {
		printf ( "ERROR: Unable to get memory.\n" );
		exit(EXIT_FAILURE);
	}
	memcpy(file_store, dir_store, strlen(dir_store));
    file_store[strlen(dir_store)] = '/';          /* separator */
    file_store[strlen(dir_store)+4+1] = '/';      /* separator */

	char *restorename = getAbsoluteRestorename(src);
	char *keyname = getAbsoluteKeyname(src);

	printf ( "RN: %s\n", restorename );
	printf ( "KN: %s\n", keyname );
	FILE *keyfile;
	keyfile = fopen(keyname, "r");
	if (keyfile == NULL) {
		printf ( "ERROR: Unable to open database-file.\n" );
		exit(EXIT_FAILURE);
	}

	/* destination (restore)file */
	FILE *restorefile;
	restorefile = fopen(restorename, "wb");
	if (!restorefile) {
		printf ( "ERROR: Unable to write file (%s).\n", restorename );
		exit(EXIT_FAILURE);
	}

	unsigned long long size_store = 0;
	size_t size;
	unsigned int count = 0;
	char line[KEY_SIZE+1];
	char buffer[BLOCK_SIZE];
	while (fgets(line, sizeof(line), keyfile) != NULL) {
		if (line[KEY_SIZE] != '\0' || strlen(line) != KEY_SIZE) continue; /* check if the line is complete */

		++count;
        printf ( "FILE-%d: %s\n", count, line );

		/* read file */
		memcpy(file_store+strlen(dir_store)+1, line, 4); /* 4 characters as directory  */
		memcpy(file_store+strlen(dir_store)+1+4+1, line, KEY_SIZE); /* md5-filename */
        file_store[strlen(dir_store)+1+4+1+KEY_SIZE] = '\0';   /* terminator */

		FILE *dataFile;
		dataFile = fopen(file_store, "rb");
		if (dataFile == NULL) {
			printf ( "\nERROR: Unable to open file (%s).\n", file_store );
			continue;
		}
		
		size = fread(buffer, 1, BLOCK_SIZE, dataFile);
		size_store += size;
		fwrite(buffer, 1, size, restorefile);

		fclose(dataFile);

        line[KEY_SIZE] = '\1';                  /* delete '\0' from string-end */

	}
	fclose(restorefile);
	printf ( "FILE-SIZE: %lld\n", size_store );
}
