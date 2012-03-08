
#ifndef COMMON_H
#define COMMON_H

#define BOOL int
#define TRUE 1
#define FALSE 0

#ifdef __unix__
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/types.h>
#else
	#include <windows.h>
	#include <direct.h>
	#define stat _stat
#endif

#define NAME "SimpleShare"
#define VERSION "0.2"
#define AUTHOR "Stephan Laukien"

#define BLOCK_SIZE 4*1024
#define KEY_SIZE 32
#define PARAMETER_KEY_SIZE 64
#define PARAMETER_VALUE_SIZE BLOCK_SIZE*2
#define PARAMETER_LINE_SIZE PARAMETER_KEY_SIZE+1+PARAMETER_VALUE_SIZE

typedef struct {
        short year;
        short month;
        short monthday;
        short weekday;
        short hour;
        short minute;
        short second;
} DATETIME;

typedef struct {
	char key[PARAMETER_KEY_SIZE+1];
	char value[PARAMETER_VALUE_SIZE+1];
	int status;
} PARAMETER;


void showCopyright();
BOOL fileExists(char *name);
char *getFilename(char *file);
BOOL directoryExists(char *name);
char *calculateSignature(char *string, size_t size); /* TODO: make it safe!!! */
DATETIME getDateTime();
char *getTimestampAsString();
char *getAbsoluteKeyname(char *filename);
char *trimString(char *string);
PARAMETER stringToParameter(char *string);
char *getHostname();
char *getAbsoluteKeyname(char *filename);
char *getAbsoluteRestorename(char *filename);
char *keyToFilename(char *key);
char *encodeString(char *string, size_t size);
char *decodeString(char *string);
#endif
