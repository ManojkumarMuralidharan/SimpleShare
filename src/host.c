/*
 * =====================================================================================
 *
 *       Filename:  host.c
 *
 *    Description:  manages the host-table
 *
 *        Version:  1.0
 *        Created:  01/20/2012 07:08:30 PM
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

void load();
void save();
ADDRESS *getAddressFromFriend(char *name, char *adr);
ADDRESS *getPingFromFriend(char *name);
void pingAllFriends();

int main(int argc, char *argv[]) {
	showCopyright("Host");
	initConfig();
	initError();
	initAddress();

	/* check if host has been locked */
	if (isLock()) {
		printf ( "WARNING: Host was locked.\n" );
		removeLock();
	}

	load();                                     /* load all hosts into memory */
	pingAllFriends();                           /* check which host is online/offline */

//		friend = getAddressFromFriend(local->name, "127.0.0.1");

	save();                                     /* save all hosts and its status */
	releaseAddress();
	releaseConfig();

	return(0);
}

void load() {
	waitLock();
	createLock();
	printf ( "LOAD: %s\n", getAddressFileAll() );

	FILE *fall;
	fall = fopen(getAddressFileAll(), "r");
	if (fall == NULL) {
		printf ( "ERROR: Unable to load addresslist (%s).\n", getAddressFileAll() );
		removeLock();
		exit(EXIT_FAILURE);
	}

	PARAMETER param;
	char line[ADDRESS_LINE_SIZE];
	while(fgets(line, sizeof(line), fall) != NULL) {
		param = stringToParameter(line);
		if (!param.status) continue;

		/* move host into memory */
		ADDRESS adr = stringToAddress(param.value);
		if (!adr.status) continue;              /* invalid entry */
		strcpy(adr.name, param.key);
		addAddress(&adr);
	}

	fclose(fall);
	removeLock();
}

void save() {
	createLock();
	printf ( "SAVE: %s, %s & %s\n", getAddressFileAll(), getAddressFileOnline(), getAddressFileOffline() );

	/* open files */
	FILE *allfile;
	allfile = fopen(getAddressFileAll(), "w");
	if (allfile == NULL) {
		printf ( "ERROR: Unable to save \"%s\".\n", getAddressFileAll() );
		removeLock();
		return;
	}

	FILE *onfile;
	onfile = fopen(getAddressFileOnline(), "w");
	if (onfile == NULL) {
		printf ( "ERROR: Unable to save \"%s\".\n", getAddressFileOnline() );
		removeLock();
		return;
	}

	FILE *offfile;
	offfile = fopen(getAddressFileOffline(), "w");
	if (offfile == NULL) {
		printf ( "ERROR: Unable to save \"%s\".\n", getAddressFileOffline() );
		removeLock();
		return;
	}

	/* write files */
	ADDRESS *adr;
	int i;
	for (i = 0; i < getAddressSize(); ++i) {
		adr = getAddressById(i);
		fprintf(allfile, "%s=%s:%d\n", adr->name, adr->host, adr->port);
		if (adr->status) fprintf(onfile, "%s\n", adr->name);
		else fprintf(offfile, "%s\n", adr->name);
	}

	/* close files */
	fclose(offfile);
	fclose(onfile);
	fclose(allfile);

	removeLock();
}

ADDRESS *getAddressFromFriend(char *name, char *adr) {
	ADDRESS address;
	strcpy(address.name, name);
	strcpy(address.host, adr);
	address.port = getConfigServerPort();

	PARAMETER command;
	strcpy(command.key, "HOST");
	strcpy(command.value, name);

	PARAMETER response;
	response = client(&address, &command);

	if (isError()) return NULL;

	if (response.status && strcmp(response.key, "ADDRESS") == 0) {
		ADDRESS tmpadr = stringToAddress(response.value);
		if (tmpadr.status) {
			strcpy(address.host, tmpadr.host);
			address.port = tmpadr.port;
			address.status = 1;

			/* get pointer to addresslist */
			ADDRESS *adrp = getAddressByName(address.name);
			if (adrp == NULL) {                         /* add new address */
				addAddress(&address);
				adrp = getAddressByName(address.name);
			}

			return adrp;
		} else {
			setError(ERROR_CLIENT_FORMAT, "invalid format");
			return NULL;
		}
	} else {
		setError(ERROR_CLIENT_NOTFOUND, "host not found");
		return NULL;
	}

}

ADDRESS *getPingFromFriend(char *name) {
	ADDRESS *address = getAddressByName(name);
	if (address == NULL) {
		setError(ERROR_CLIENT_NOTFOUND, "friend not found");
		return NULL;
	}

	PARAMETER command;
	strcpy(command.key, "PING");
	strcpy(command.value, getConfigClientName());

	PARAMETER response;
	response = client(address, &command);

	if (isError()) return NULL;

	if (response.status && strcmp(response.key, "PONG") == 0) {
		if (strcmp(address->name, response.value) == 0) {
			return address;
		} else {
			char *msg = (char *)malloc(14+strlen(response.value)+1+1);
			sprintf(msg, "invalid name (%s)",response.value);
			setError(ERROR_CLIENT_NAME, msg);
			free(msg);
			address->status = 0;
			return address;
		}
	} else {
		address->status = 0;
		setError(ERROR_CLIENT_NOTFOUND, "host not found");
		return address;
	}

	/* get pointer to addresslist */
	ADDRESS *adrp = getAddressByName(name);
	return adrp;
}

void pingAllFriends() {
	ADDRESS *local, *ping;
	int i;
	for (i = 0; i < getAddressSize(); ++i) {
		clearError();
		local = getAddressById(i);
		if (!local->status) continue;

		printf ( "PING: %s ==> ", local->name );
		ping = getPingFromFriend(local->name);

		if (isError()) {
			printf ( "FAILURE\n" );
			local->status = 0;
		} else {
			printf ( "SUCCESS\n" );
			local->status = 1;
		}
	}
}
