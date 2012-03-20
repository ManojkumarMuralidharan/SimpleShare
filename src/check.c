/*
 * =====================================================================================
 *
 *       Filename:  check.c
 *
 *    Description:  checks if all parts are available and it's checksum
 *
 *        Version:  1.0
 *        Created:  01/24/2012 03:08:30 PM
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

void check(char *src);

int main(int argc, char *argv[]) {
	showCopyright("Check");
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

	check(filename);

	releaseConfig();
	return(EXIT_SUCCESS);
}

void check(char *src) {
	/* prepare filename (store/1234/... */
	char *dir_store = getConfigStore();
	int dir_len = strlen(dir_store) + 1 + 4 + 1 + 32 + 1;
	char *file_store;
	file_store = (char*)malloc(dir_len);
	if (file_store == NULL) {
		printf ( "ERROR: Unable to get memory.\n" );
		exit(EXIT_FAILURE);
	}
	memcpy(file_store, dir_store, strlen(dir_store));
    file_store[strlen(dir_store)] = '/';          /* separator */
    file_store[strlen(dir_store)+4+1] = '/';      /* separator */

	char *keyname = getAbsoluteKeyname(src);

	FILE *keyfile;
	keyfile = fopen(keyname, "rb");
	if (keyfile == NULL) {
		printf ( "ERROR: Unable to open database-file.\n" );
		exit(EXIT_FAILURE);
	}

	size_t size;
	char line[32+1];
	char signature[32+1];	
	char buffer[BLOCK_SIZE+1];
	while (fgets(line, sizeof(line), keyfile) != NULL) {
		if (line[32] != '\0' || strlen(line) != 32) continue; /* check if the line is complete */

		printf ( "%s:\t", line );               /* print hash */

		/* read file */
		memcpy(file_store+strlen(dir_store)+1, line, 4);
		memcpy(file_store+strlen(dir_store)+1+4+1, line, 32);
        file_store[strlen(dir_store)+1+4+1+32] = '\0';   /* terminator */

		/* file exists */
		struct stat st;
		if (stat(file_store, &st) != 0) {
			printf ( "FAILUE\tdoesn't exists\n" );
			continue;
		}
		FILE *dataFile;
		dataFile = fopen(file_store, "r");
		if (dataFile == NULL) {
			printf ( "FAILUE\tunable to open\n" );
			continue;
		}
		
		memset(buffer, '\0', BLOCK_SIZE+1);         /* initial clearing */
		size = fread(buffer, 1, BLOCK_SIZE, dataFile);
		strcpy(signature, calculateSignature(buffer, size));

		fclose(dataFile);

		if (strcmp(line, signature) == 0) printf ( "SUCCESS\n" );
		else printf ( "FAILUE\twrong signature\n" );

		line[32] = '\1';                        /* delete '\0' from string-end */
	}

	fclose(keyfile);
	free(file_store);
	free(keyname);
}
