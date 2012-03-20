/*
 * =====================================================================================
 *
 *       Filename:  server.c
 *
 *    Description:  common share-server
 *
 *        Version:  1.0
 *        Created:  02/28/2012 07:34:15 PM
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
#include "address.h"
#include "la_file.h"
#include "la_string.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_QUEUE 1

void server();
PARAMETER command_host(int sock_2, char *value);
PARAMETER command_ping(int sock_2, char *value);
PARAMETER command_download(int sock_2, char *value);

int main(int argc, char *argv[]) {
	showCopyright("Server");
	initConfig();

    int sock_1, sock_2;                         /* file discriptors for sockets */

	char buf[PARAMETER_LINE_SIZE+1];
	struct sockaddr_in server;

	/* create stream socket in internet domain */
	sock_1 = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_1 < 0) {
		perror("open stream socket");
		exit(EXIT_FAILURE);
	}

	memset(&server, 0, sizeof(server));         /* reset struct */

	server.sin_family = AF_INET;                /* build address in internet domain */
	server.sin_addr.s_addr = INADDR_ANY;        /* everyone is allowedto connect to server */
	server.sin_port = htons(getConfigServerPort());

	/* bind socket */
	if (bind(sock_1, (struct sockaddr *) &server, sizeof(struct sockaddr_in))) {
		perror("bind socket to server_addr");
		exit(EXIT_FAILURE);
	}

	listen(sock_1, MAX_QUEUE);
//	int tr = 1;
//	if (setsockopt(sock_1, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int)) == -1) {
//		perror("setsockopt");
//		exit(EXIT_FAILURE);
//	}

	PARAMETER command;
	while(1) {
        sock_2 = accept(sock_1, 0, 0);          /* start accepting connection */
		if (sock_2 < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		/* read from sock_2 */
		int rec_value = read(sock_2, buf, ADDRESS_LINE_SIZE);
		if (rec_value < 0) {
			perror("reading stream message");
			exit(EXIT_FAILURE);
		}
		buf[rec_value] = '\0';                  /* end string */

        command = stringToParameter(buf);       /* get parameter */
		if (!command.status) {
			printf ( "ERROR: invalid format.\n" );
			exit(EXIT_FAILURE);
		}

		PARAMETER response;
		response.status = 0;

		if (strcmp(command.key, "HOST") == 0) response = command_host(sock_2, command.value);
		else if (strcmp(command.key, "PING") == 0) response = command_ping(sock_2, command.value);
		else if (strcmp(command.key, "DOWNLOAD") == 0) response = command_download(sock_2, command.value);
		else {
			printf ( "ERROR: Invalid command (%s).\n", command.key );
			setError(ERROR_SERVER_COMMAND, "invalid command");
			strcpy(response.key, "ERROR");
			strcpy(response.value, "invalid command");
			response.status = 1;
		}

		/* write buffer to stream socket */
		if (response.status) {
//			int keylen = strlen(response.key);
//			int valuelen = (response.status == 1 ? strlen(response.value) : response.status);
//			memcpy(buf, response.key, keylen);
//			buf[keylen] = '=';
//			memcpy(buf + 1 + keylen, response.value, valuelen );
			sprintf(buf, "%s=%s", response.key, response.value);

			int total = 0;
			int byteleft = strlen(buf);
			int n = 0;

			while ( total < strlen(buf)) {
				n = send(sock_2, buf+total, byteleft, 0);
				if (n == -1) {
					perror("write on stream socket");
					exit(EXIT_FAILURE);
				}

				total += n;
				byteleft -= n;
			}
		}

		close(sock_2);
	}

	close(sock_1);
	releaseConfig();
}

PARAMETER command_host(int sock_2, char *value) {
	PARAMETER response;
	response.status = 0;

	char *online_str = (char *)malloc(strlen(getConfigHome()) + 12 +1);
	sprintf(online_str, "%s/host.online", getConfigHome());

	FILE *online_file;
	online_file = fopen(online_str, "r");
	if (online_file == NULL) return response;

	char line[ADDRESS_LINE_SIZE+1];
	char *linep = &line[0];
	while (fgets(line, ADDRESS_LINE_SIZE, online_file) != NULL) {
		linep = string_trim(linep);
		if (strlen(line) == 0) continue;
		if (strcmp(line, value) != 0) continue;

		/* set response */
		response.status = 1;

		break;                          /* entry found */
	}

	/* free resources */
	if (online_file != NULL) {
		fclose(online_file);
		online_file = NULL;
	}
	if (online_str != NULL) {
		free(online_str);
		online_str = NULL;
	}

	if (response.status) {
		/* read address from host.all */
		ADDRESS adr  = readAddress(value);
		if (adr.status) {
			strcpy(response.key, "ADDRESS");
			sprintf(response.value, "%s:%d", adr.host, adr.port );
		} else {
			strcpy(response.key, "ERROR");
			strcpy(response.value, "Host not found.");
		}
	} else {
		strcpy(response.key, "ERROR");
		strcpy(response.value, "Host not online.");
		response.status = 1;
	}

	return response;
}

PARAMETER command_ping(int sock_2, char *value) {
	PARAMETER response;
	response.status = 1;
	strcpy(response.key, "PONG");
	strcpy(response.value, getConfigClientName());

	return response;
}

PARAMETER command_download(int sock_2, char *value) {
	PARAMETER response;
	response.status = 1;

	/* check size */
	if (strlen(value) != KEY_SIZE) {
		strcpy(response.key, "ERROR");
		strcpy(response.value, "invalid key-size");
		return response;
	}

	/* check if file exists */
	char *filename = keyToFilename(value);
	if (!file_exists(filename)) {
		free(filename);
		strcpy(response.key, "ERROR");
		strcpy(response.value, "file not found");
		return response;
	}

	/* read and send file */
	FILE *file;
	file = fopen(filename, "rb");
	if (!filename) {
		printf ( "ERROR: Unable to read file (%s).\n", filename );
		exit(EXIT_FAILURE);
	}

	/* read block */
	char buffer[BLOCK_SIZE+1];
	int size = fread(buffer, 1, BLOCK_SIZE, file);
	strcpy(response.key, "FILE");
//	memcpy(response.value, buffer, size);

	/* encode data */
	char *buffer_enc = encodeString(buffer, size);
	strcpy(response.value, buffer_enc);
	free(buffer_enc);

	fclose(file);
	free(filename);

	return response;
}
