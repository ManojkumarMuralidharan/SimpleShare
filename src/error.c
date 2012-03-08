#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int _id;
char _msg[ERROR_MESSAGE_SIZE+1];

void initError() {
	clearError();
}

void setError(int id, char *msg) {
	if (strlen(msg) > ERROR_MESSAGE_SIZE) {
		printf ( "ERROR: Message to big.\n" );
		exit(EXIT_FAILURE);
	}

	_id =id;
	strcpy(_msg, msg);
}

int getErrorId() {
	return _id;
}

char *getErrorMessage() {
	if (!isError()) return NULL;

	return _msg;
}

BOOL isError() {
	return (_id != ERROR_NONE || strlen(_msg) > 0);
}

void clearError() {
	_id = ERROR_NONE;
	_msg[0] = '\0';
}

