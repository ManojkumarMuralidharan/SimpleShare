/*
 * =====================================================================================
 *
 *       Filename:  download.c
 *
 *    Description:  get files from servers
 *
 *        Version:  1.0
 *        Created:  03/04/2012 12:04:30 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Stephan Laukien (slaukien), software@laukien.com
 *        Company:  
 *
 * =====================================================================================
 */

#include "common.h"
#include "error.h"
#include "config.h"
#include "address.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DOWNLOAD_STATUS 5

char *file_item;
int file_last;

void loadOnline();
void download(char *src);

int main(int argc, char *argv[]) {
	showCopyright("Download");
	initConfig();
	initError();
	initAddress();

	/* check arguments */
	if (argc != 2) {
		printf ( "ERROR: Invalid arguments.\n" );
		printf ( "\t%s DATABASE-FILE\n", argv[0] );
		exit(EXIT_FAILURE);
	}

	char *filename = argv[1];
	char *filename_abs = getAbsoluteKeyname(filename);
	if (!fileExists(filename_abs)) {
		free(filename_abs);
		printf ( "ERROR: File (%s) doesn't exists.\n", filename );
		exit(EXIT_FAILURE);
	}

	/* load online-friends into memory */
	loadOnline();

	download(filename_abs);

	free(filename_abs);
	
	releaseAddress();
	releaseConfig();

	return(EXIT_SUCCESS);
}

void loadOnline() {
	printf ( "LOAD: %s\n", getAddressFileOnline() );

	/* wait until host isn't locked */
	waitLock();

	FILE *file;
	file = fopen(getAddressFileOnline(), "r");
	if (file == NULL) {
		printf ( "ERROR: Unable to open online-list.\n" );
		removeLock();
		exit(EXIT_FAILURE);
	}

	char line[ADDRESS_KEY_SIZE];
	while(fgets(line, ADDRESS_KEY_SIZE, file) != NULL) {
		trimString(line);
		ADDRESS *adr;
        adr = getAddressByName(line);           /* load online-friends into memory */
        adr->status = DOWNLOAD_STATUS;          /* set the starting status which means how many tries will be done */
	}

	fclose(file);
	removeLock();
}

void download(char *src) {
	FILE *file;
	file = fopen(src, "r");
	if (file == NULL) {
		printf ( "ERROR: Unable to open key-file.\n" );
		exit(EXIT_FAILURE);
	}

	char *keyfilename;
	char key[KEY_SIZE+1];
	unsigned long long count = 0;
	while (fgets(key, KEY_SIZE+1, file) != NULL) {
		trimString(key);
		key[KEY_SIZE] = '\0';
		if (strlen(key) != KEY_SIZE) continue;

		keyfilename = keyToFilename(key);
		printf ( "FILE-%lld: %s\t", ++count, key );

		if (fileExists(keyfilename)) {
			printf ( "EXISTS\n" );
			free(keyfilename);                  /* free memory */
			continue;
		}

		free(keyfilename);                      /* free memory */

		/* download file */
		printf ( "DOWNLOAD\n" );
		PARAMETER cmd;
		strcpy(cmd.key, "DOWNLOAD");
		strcpy(cmd.value, key);

		ADDRESS *adr = getAddressByRandom();
		do {
			if (adr->status == 0) continue;
			clearError();
			PARAMETER response;
			response = client(adr, &cmd);

			if (isError() || strcmp(response.key, "FILE") != 0) {
				printf ( "ERROR: %s (%d)\n", getErrorMessage(), getErrorId() );
				--(adr->status);                /* decrement status */
			}
		} while ((adr = getAddressByRandom()) != NULL && isError());

		if (isError()) {
			printf ( "ERROR: Unable to get file (%s).\n", key );
		}

		/* check file */

		/* safe file */
	}

	fclose(file);
}
