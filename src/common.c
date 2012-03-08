
#include "common.h"
#include "error.h"
#include "md5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>

extern char *getConfigHome();                   /* from config */
extern int getConfigServerPort();               /* from config */
extern char *getConfigKey();                    /* from config */
extern char *getConfigStore();                  /* from config */
extern char *getConfigRestore();                /* from config */

char hostname[1024];

void showCopyright(char *prg) {
	printf ( "\n%s - %s v.%s (%s)\n", NAME, prg, VERSION, __DATE__ );
	printf ( "(c) 2011-2012 by %s\n\n", AUTHOR );
}

BOOL fileExists(char *name) {
	struct stat st;
	return (stat(name, &st) == 0);
}

BOOL directoryExists(char *name) {
	struct stat st;
	return (stat(name, &st) == 0);

}

char *calculateSignature(char *string, size_t size) {
	static char signature[MD5_SIZE*2+1];

	unsigned char sig[MD5_SIZE];

	md5_buffer(string, size, sig);
	
	const unsigned char *sig_p;
	int i = 0;
	for (sig_p = sig; sig_p < sig + MD5_SIZE; sig_p++) {
		sprintf(signature+(i*2), "%02x", *sig_p);
		++i;
	}

//	printf ( "SIG: %s %s\n", string, signature );

	return signature;
}

DATETIME getDateTime() {
        DATETIME now;
        time_t ltime;
        struct tm *Tm;

        ltime=time(NULL);
        Tm=localtime(&ltime);

        now.year=Tm->tm_year+1900;
        now.month=Tm->tm_mon+1;
        now.monthday=Tm->tm_mday;
        now.weekday=Tm->tm_wday;
        now.hour=Tm->tm_hour;
        now.minute=Tm->tm_min;
        now.second=Tm->tm_sec;

        return now;
}


char *getTimestampAsString() {

	char *date;
	date = malloc (15);
	if ( date==NULL ) {
		fprintf ( stderr, "\ndynamic memory allocation failed\n" );
		exit (EXIT_FAILURE);
	}

	DATETIME now = getDateTime();
	sprintf(date, "%4d%02d%02d%02d%02d%02d", now.year, now.month, now.monthday, now.hour, now.minute, now.second);

	return date;
}

char *getFilename(char *file) {
	int i;
	for (i = strlen(file)-1; i >= -1; i--) {
		if (i == -1) break;                     /* not found */
		if (file[i] == '/') break;
	}

	int l = strlen(file) - i;                   /* len + 1 */
	char *name = malloc(l);
	if(name == NULL) {
		fprintf ( stderr, "\ndynamic memory allocation failed\n" );
		exit (EXIT_FAILURE);
	}

	memcpy(name, file + i + 1, l);              /* name + '\0' */

	return name;
}

char *trimString(char *string) {
	int len = strlen(string);

	int s;
	for (s=0; s<len; s++) {
		if(isalnum(string[s])) break;
	}

	int e;
	for (e=len-1; e>s; e--) {
		if(isalnum(string[e])) break;
	}

	if (s == 0 && e == len-1) return string;    /* nothing to change */

	if (s != 0) {
//		int i;
//		for (i=0; i<len-s; i++) {
//			string[i] = string[i+s];
//		}
		memcpy(string, string+s, len-s);
		string[len-s] = '\0';
	}

	if (e != len-1) {
		string[e-s+1] = '\0';
	}

	return string;
}

PARAMETER stringToParameter(char *string) {
	/* set parameters */
	PARAMETER param;
	param.key[0] = '\0';
	param.value[0] = '\0';
    param.status = 0;                           /* parameter invalid */

	trimString(string);

    if (strlen(string) < 3) return param;       /* invalid */
    if (strchr(string, '#')) return param;      /* comment */

	char *sep = strchr(string, '=');
    if (!sep) return param;                     /* no entry */

	/* check parameter-size */
	int keyl = sep-string;
	if (keyl > PARAMETER_KEY_SIZE) return param;
	int valuel = strlen(string)-1-keyl;
	if (valuel > PARAMETER_VALUE_SIZE) return param;

	/* key */
	memcpy(param.key, string, keyl);
	param.key[keyl] = '\0';
	
	/* value */
	memcpy(param.value, sep+1, valuel);
	param.value[valuel] = '\0';


	param.status = 1;
	return param;
}

char *getHostname() {
	size_t size = sizeof(hostname)/sizeof(hostname[0]);
	memset(hostname, '\0', size);
	gethostname(hostname, size-1);

	return hostname;
}

char *getAbsoluteKeyname(char *filename) {
	char *filename_abs;
	filename_abs = (char*)malloc(strlen(getConfigKey())+1+strlen(filename)+1);
	if (filename_abs == NULL) {
		fprintf ( stderr, "\ndynamic memory allocation failed\n" );
		exit (EXIT_FAILURE);
	}
	sprintf(filename_abs, "%s/%s", getConfigKey(), filename);
	
	return(filename_abs);
}

char *getAbsoluteRestorename(char *filename) {
	int tmpsize = strlen(filename)-14-1-4;
	if (tmpsize <= 1) {
		printf ( "ERROR: Invalid key-file.\n" );
		exit(EXIT_FAILURE);
	}

	char *tmpname = (char*)malloc(tmpsize+1);
	if (tmpname == NULL) {
		printf ( "ERROR: Unable to get memory.\n" );
		exit(EXIT_FAILURE);
	}
	memcpy(tmpname, filename, tmpsize);
	tmpname[tmpsize] = '\0';

	char *filename_abs;
	filename_abs = (char*)malloc(strlen(getConfigRestore())+1+strlen(tmpname)+1);
	if (filename_abs == NULL) {
		fprintf ( stderr, "\ndynamic memory allocation failed\n" );
		exit (EXIT_FAILURE);
	}
	sprintf(filename_abs, "%s/%s", getConfigRestore(), tmpname);

	free(tmpname);
	return(filename_abs);
}

char *keyToFilename(char *key) {
	if (strlen(key) != KEY_SIZE) {
		printf ( "ERROR: Invalid KEY-size.\n" );
		exit(EXIT_FAILURE);
	}

	//~/.SimpleShare/store/1234/KEY

	/* get memory for key-file */
	int dirsize = strlen(getConfigStore());
	int filesize = dirsize + 1 + 4 + 1 + KEY_SIZE + 1;
	char *filename = malloc(filesize + 1);
	if (filename == NULL) {
		printf ( "ERROR: Unable to get memory.\n" );
		exit(EXIT_FAILURE);
	}

	memcpy(filename, getConfigStore(), dirsize);
	filename[dirsize] = '/';
	memcpy(filename + dirsize + 1, key, 4);
	filename[dirsize + 4 + 1] = '/';
	memcpy(filename + dirsize + 1 + 4 + 1, key, KEY_SIZE);
	filename[filesize] = '\0';

	return filename;
}

char *encodeString(char *string, size_t size) {
	char hex[] = "0123456789abcdef";
	char *result = malloc((size*2)+1);
	if (result == NULL) {
		printf ( "ERROR: Unable to get memory.\n" );
		exit(EXIT_FAILURE);
	}
	result[size*2] = '\0';                      /* finish string */

	int i;
	for (i = 0; i < size; ++i) {
		result[i << 1] = hex[string[i] >> 4 & 0x0f];
		result[(i << 1) + 1] = hex[string[i] & 0x0f];
	}

	return result;
}

char *decodeString(char *string) {
	int len = strlen(string)/2+1;
	char *result = malloc(len);
	if (result == NULL) {
		printf ( "ERROR: Unable to get memory.\n" );
		exit(EXIT_FAILURE);
	}
    result[len] = '\0';                         /* finish string */

	char buf[3];
	buf[2] = '\0';
	int i;
	unsigned long c;
	for (i = 0; i < strlen(string); i += 2) {
		buf[0] = string[i];
		buf[1] = string[i+1];
		c = strtoul(buf, NULL, 16);
		result[i/2] = (char)c;
	}

	return result;
}

