/*
 * =====================================================================================
 *
 *       Filename:  address.c
 *
 *    Description:  library which helps to work with in-memory-addresses
 *
 *        Version:  1.0
 *        Created:  03/04/2012 12:16:30 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Stephan Laukien (slaukien), software@laukien.com
 *        Company:  
 *
 * =====================================================================================
 */

#include "address.h"
#include "common.h"
#include "error.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

char *file_all;
char *file_lock;
char *file_online;
char *file_offline;

ADDRESS *all_item;
int all_last;

void initAddress() {
	int path_len = strlen(getConfigHome()) + 1; /* ~/.SimpleShare/ */

	/* all */
	file_all = malloc(path_len+strlen(ADDRESS_FILE_ALL)+1);
	if (file_all == NULL) {
		printf ( "ERROR: Unable to get memory.\n" );
		exit(EXIT_FAILURE);
	}
	sprintf(file_all, "%s/%s", getConfigHome(), ADDRESS_FILE_ALL);

	/* lock */
	file_lock = malloc(path_len+strlen(ADDRESS_FILE_LOCK)+1);
	if (file_lock == NULL) {
		printf ( "ERROR: Unable to get memory.\n" );
		exit(EXIT_FAILURE);
	}
	sprintf(file_lock, "%s/%s", getConfigHome(), ADDRESS_FILE_LOCK);

	/* online */
	file_online = malloc(path_len+strlen(ADDRESS_FILE_ONLINE)+1);
	if (file_online == NULL) {
		printf ( "ERROR: Unable to get memory.\n" );
		exit(EXIT_FAILURE);
	}
	sprintf(file_online, "%s/%s", getConfigHome(), ADDRESS_FILE_ONLINE);

	/* offline */
	file_offline = malloc(path_len+strlen(ADDRESS_FILE_OFFLINE)+1);
	if (file_offline == NULL) {
		printf ( "ERROR: Unable to get memory.\n" );
		exit(EXIT_FAILURE);
	}
	sprintf(file_offline, "%s/%s", getConfigHome(), ADDRESS_FILE_OFFLINE);
	/* init list of all hosts */
	all_item = (ADDRESS *)malloc (sizeof(ADDRESS)*10);
	if (all_item == 0) {
		printf ( "ERROR: Unable to get memory.\n" );
		exit(EXIT_FAILURE);
	}

	all_last = 0;                               /* no items */

	/* init random */
	time_t t;
	t = time(NULL);
	srand(t);
}

void releaseAddress() {
	free(file_all);
	file_all = NULL;

	free(file_lock);
	file_lock = NULL;

	free(file_online);
	file_online = NULL;

	free(file_offline);
	file_offline = NULL;

	free(all_item);
	all_item = NULL;
}

BOOL isLock() {
	return (fileExists(file_lock));
}

void createLock() {
	FILE *file;
	file = fopen(file_lock, "w");
	if (file == NULL) {
		printf ( "ERROR: Unable to create lock.\n" );
		exit(1);
	}

	fprintf(file, "HOST-LOCK");

	fclose(file);
}

void removeLock() {
	if (!fileExists(file_lock)) return;

	int error = remove(file_lock);
	if (error) {
		printf ( "ERROR: Unable to remove lock.\n" );
		exit(1);
	}
}

void waitLock() {
	while (isLock()) {
		printf ( "INFO: Host is locked.\n" );
		sleep (100);
	}
}

char *getAddressFileAll() {
	return file_all;
}

char *getAddressFileLock() {
	return file_lock;
}

char *getAddressFileOnline() {
	return file_online;
}

char *getAddressFileOffline() {
	return file_offline;
}

void addAddress(ADDRESS *adr) {
	memcpy(all_item+all_last, adr, sizeof(ADDRESS));
	++all_last;                                 /* add counter */

	/* get more memory if need */
	if (all_last%10 == 0) {
		all_item = (ADDRESS *)realloc(all_item, sizeof(ADDRESS)*(all_last+10));
		if (all_item == NULL) {
			printf ( "ERROR: Unable to get memory.\n" );
			exit(EXIT_FAILURE);
		}
	}

	printf ( "HOST %d: %s => %s:%d\n", all_last, adr->name, adr->host, adr->port );
}

size_t getAddressSize() {
	return all_last;
}

ADDRESS *getAddressById(int idx) {
	ADDRESS *adr;

	if (idx < 0 || idx >= all_last) return NULL;

	adr = all_item+idx;

	return adr;
}

ADDRESS *getAddressByName(char *name) {
	ADDRESS *adr;

	/* read in memory */
	int i;
	for(i = 0; i < all_last; ++i) {
		adr = all_item + i;
		if (strcmp(name, adr->name) == 0) return adr;
	}

	/* read in file */
	FILE *file;
	file = fopen(getAddressFileAll(), "r");
	if (file == NULL ) {
		printf ( "ERROR: No Address found.\n" );
		exit(EXIT_FAILURE);
	}

	char line[ADDRESS_LINE_SIZE+1];
	while (fgets(line, ADDRESS_LINE_SIZE, file) != NULL) {
		trimString(line);
		if (strstr(line, name) != NULL) {
			ADDRESS tmp = stringToAddress(line);
			addAddress(&tmp);
			break;
		}
	}

	fclose(file);

	/* get pointer to address */
	for(i = 0; i < all_last; ++i) {
		adr = all_item + i;
		if (strcmp(name, adr->name) == 0) return adr;
	}

	return NULL;
}

ADDRESS *getAddressByRandom() {
	int idx = rand() % all_last;
	return getAddressById(idx);
}

ADDRESS stringToAddress(char *string) {
	ADDRESS address;

	/* check length of string */
	if (strlen(string) > NAME_SIZE + 1 + HOST_SIZE + 1 + 5) {
		address.status = 0;
		return address;
	}

	address.status = 1;

	int len;
	char *sep;
	sep = strchr(string, '=');
    if (sep == NULL) {                          /* no name */
		address.name[0] = '\0';
	} else {
		len = strlen(string) - strlen(sep);     /* length of name */
		memcpy(address.name, string, len);
		address.name[len] = '\0';
		string = sep+1;                         /* string is now the host-port-part */
	}

	sep = strchr(string, ':');
    if (sep == NULL) {                          /* no port */
		strcpy(address.host, string);
		address.port = getConfigServerPort();
		return address;
	}

    len = strlen(string) - strlen(sep);     /* length of host */
	memcpy(address.host, string, len);
	address.host[len] = '\0';

    len = strlen(string) - len - 1;             /* length of port */
	if (len > 5) {
		address.port = -1;
		address.status = 0;                     /* mark it as invalid */
	} else {
		char port[6];
		memcpy(port, sep+1, len);
		port[5] = '\0';
		address.port = atoi(port);
		if (address.port <= 0) address.status = 0; /* mark it as invalid */
	}

	return address;
}

void showAddress(ADDRESS *adr) {

	if (adr == NULL) {
		printf ( "No address given.\n" );
		return;
	}
	printf ( "Name: %s\n", adr->name );
	printf ( "Host: %s\n", adr->host );
	printf ( "Port: %d\n", adr->port );
	printf ( "Stat: %d\n", adr->status );
}

ADDRESS readAddress(char *name) {
	ADDRESS adr;
	adr.status = 0;
	strcpy(adr.name, name);

	char *filename = (char *)malloc(strlen(getConfigHome()) + 9 +1);
	sprintf(filename, "%s/host.all", getConfigHome());

	FILE *file;
	file = fopen(filename, "r");
	if (file == NULL) {
		adr.status = 0;
		return adr;
	}

	ADDRESS tmp;
	char line[ADDRESS_LINE_SIZE+1];
	while (fgets(line, ADDRESS_LINE_SIZE, file) != NULL) {
		tmp = stringToAddress(line);
		if (!tmp.status) continue;

		if (strcmp(tmp.name, name) == 0) {
			strcpy(adr.host, tmp.host);
			adr.port = tmp.port;
			adr.status = 1;
			break;
		}
	}
	fclose(file);

	return adr;
}

PARAMETER client(ADDRESS *address, PARAMETER *command) {
	PARAMETER param;
	param.status = 0;

	int sock;
	char buf[PARAMETER_LINE_SIZE+1];
	struct sockaddr_in server;
	struct hostent *hp;

	/* create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		setError(ERROR_CONNECTION_OPEN, "open stream socket");
		return param;
	}

	memset(&server, 0, sizeof(server));         /* reset struct */

	server.sin_family = AF_INET;

	/* get internet address of host specified by command line*/
	hp = gethostbyname(address->host);
	if (hp == NULL) {
		setError(ERROR_CONNECTION_OPEN, "unknown host");
		return param;
	}

	bcopy(hp->h_addr, &server.sin_addr, hp->h_length); /* copies the internet address to server address */
    server.sin_port = htons(address->port);     /* set port */

	/* open connection */
	if (connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) < 0) {
		setError(ERROR_CONNECTION_OPEN, "connecting stream socket");
		return param;
	}

	sprintf(buf, "%s=%s", command->key, command->value);
	/* write buffer to stream socket */
	if (write(sock, buf, strlen(buf)) < 0) {
		setError(ERROR_CONNECTION_WRITE, "write on stream socket");
		close(sock);
		return param;
	}

//	int rec_value = read(sock, buf, PARAMETER_LINE_SIZE);
//	if (rec_value < 0) {
//		setError(ERROR_CONNECTION_READ, "reading stream message");
//		close(sock);
//		return param;
//	}

	int total = 0;
	int n = 1;                                  /* dummy value greater than 0 */

	while (n > 0 && total <= PARAMETER_LINE_SIZE) {
		n = recv(sock, buf+total, PARAMETER_LINE_SIZE, 0);
		if (n == -1) {
			setError(ERROR_CONNECTION_READ, "reading stream message");
			break;
		}
		total += n;
	}
    buf[total] = '\0';                          /* end string */

	close(sock);

	param = stringToParameter(buf);
	param.status = 1;

	return param;
}
