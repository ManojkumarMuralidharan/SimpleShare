#ifndef ADDRESS_H
#define ADDRESS_H

#include "common.h"
#include "error.h"

#define ADDRESS_FILE_ALL "host.all"
#define ADDRESS_FILE_LOCK "host.lock"
#define ADDRESS_FILE_ONLINE "host.online"
#define ADDRESS_FILE_OFFLINE "host.offline"

#define NAME_SIZE 64
#define HOST_SIZE 64
#define ADDRESS_KEY_SIZE 64
#define ADDRESS_VALUE_SIZE 64
#define ADDRESS_LINE_SIZE ADDRESS_KEY_SIZE+1+ADDRESS_VALUE_SIZE

typedef struct {
	char name[NAME_SIZE+1];
	char host[HOST_SIZE+1];
	int port;
	int status;
} ADDRESS;

void initAddress();
void releaseAddress();
BOOL isLock();
void waitLock();
void createLock();
void removeLock();
char *getAddressFileAll();
char *getAddressFileLock();
char *getAddressFileOnline();
char *getAddressFileOffline();
void addAddress(ADDRESS *adr);
size_t getAddressSize();
ADDRESS *getAddressById(int idx);
ADDRESS *getAddressByName(char *name);
ADDRESS *getAddressByRandom();
ADDRESS stringToAddress(char *string);
void showAddress(ADDRESS *adr);
ADDRESS readAddress(char *name);
PARAMETER client(ADDRESS *address, PARAMETER *command);

#endif
