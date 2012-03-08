#ifndef ERROR_H
#define ERROR_H

#include "common.h"

#define ERROR_MESSAGE_SIZE 1024

#define ERROR_NONE 0
#define ERROR_UNKNOWN -1

#define ERROR_CONNECTION 32
#define ERROR_CONNECTION_OPEN ERROR_CONNECTION+1
#define ERROR_CONNECTION_READ ERROR_CONNECTION+2
#define ERROR_CONNECTION_WRITE ERROR_CONNECTION+3

#define ERROR_CLIENT 128
#define ERROR_CLIENT_NOTFOUND ERROR_CLIENT+1
#define ERROR_CLIENT_ADDRESS ERROR_CLIENT+2
#define ERROR_CLIENT_FORMAT ERROR_CLIENT+3
#define ERROR_CLIENT_NAME ERROR_CLIENT+4

#define ERROR_SERVER 256
#define ERROR_SERVER_COMMAND ERROR_SERVER+1


void initError();
void setError(int id, char *msg);
int getErrorId();
char *getErrorMessage();
void showError();
BOOL isError();
void clearError();

#endif
