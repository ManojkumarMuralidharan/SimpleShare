#include "common.h"
#include "config.h"
#include "la_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *client_name;
char *file_config;
char *dir_config;
char *dir_store;
char *dir_restore;
char *dir_key;
int server_port;

void loadConfig();
void saveConfig();

void initConfig() {
	client_name = NULL;
	file_config = NULL;
	dir_store = NULL;
	dir_restore = NULL;
	dir_key = NULL;
	server_port = CONFIG_SERVER_PORT;
	
	char *home = getenv("HOME");

	if (home == NULL) {
		printf ( "ERROR: Home-directory not found.\n" );
		exit(EXIT_FAILURE);
	}

	/* dir_config */
	dir_config = malloc (strlen(home)+1+strlen(CONFIG_DIRECTORY)+1);
	if (dir_config == NULL) {
		fprintf ( stderr, "\ndynamic memory allocation failed\n" );
		exit (EXIT_FAILURE);
	}
	sprintf(dir_config, "%s/%s", home, CONFIG_DIRECTORY);
	mkdir(dir_config, 0755);
	printf ( "HOME: %s\n", dir_config );

	/* file_config */
	file_config = malloc (strlen(dir_config)+1+strlen(CONFIG_FILE)+1);
	if (file_config == NULL) {
		fprintf ( stderr, "\ndynamic memory allocation failed\n" );
		exit (EXIT_FAILURE);
	}
	sprintf(file_config, "%s/%s", dir_config, CONFIG_FILE);
	printf ( "CONFIG: %s\n", file_config );

	loadConfig();

	if (dir_store == NULL) setConfigStore(NULL);
	if (dir_restore == NULL) setConfigRestore(NULL);
	if (dir_key == NULL) setConfigKey(NULL);
	if (client_name == NULL) setConfigClientName(NULL);
	if (server_port == -1) setConfigServerPort(-1);

	/* check if config already exists */
	if (!file_exists(file_config)) {
		saveConfig();
	}

}

void releaseConfig() {

	saveConfig();

	/* file_config */
	if (file_config != NULL) {
		free(file_config);
		file_config = NULL;
	}

	/* dir_config */
	if (dir_config != NULL) {
		free(dir_config);
		dir_config = NULL;
	}

	/* dir_store */
	if (dir_store != NULL) {
		free(dir_store);
		dir_store = NULL;
	}

	/* dir_restore */
	if (dir_restore != NULL) {
		free(dir_restore);
		dir_restore = NULL;
	}

	/* dir_key */
	if (dir_key != NULL) {
		free(dir_key);
		dir_key = NULL;
	}

	/* client_name */
	if (client_name != NULL) {
		free(client_name);
		client_name = NULL;
	}

	/* sever_port */
	server_port = -1;

}

void loadConfig() {
	char key[PARAMETER_KEY_SIZE+1];
	char value[PARAMETER_VALUE_SIZE+1];
	char line[PARAMETER_LINE_SIZE+1];
	int len;

	if (file_config == NULL) {
		printf ( "ERROR: Config-file not set.\n" );
		exit(EXIT_FAILURE);
	}

	FILE *file;
	file = fopen(file_config, "r");
	if (file == NULL) {
		printf ( "WARNING: Unable to read config-file (%s).\n", file_config );
		return;
	}

	while (fgets(line, sizeof(line), file) != NULL) {
		if(strlen(line) < 5 || line[0] == '#') continue;
		line[strlen(line)-1] = '\0';            /* delete line-break */
		char *idx = strchr(line, '=');
		if (idx == NULL) {
			printf ( "ERROR: Invalid config-file.\n" );
			exit(EXIT_FAILURE);
		}

		len = idx-line;
		if (len <= PARAMETER_KEY_SIZE) {
			memcpy(key, line, len);
			key[len] = '\0';
		}

		len = strlen(line)-(idx-line)-1;

		if (len <= PARAMETER_VALUE_SIZE) {
			memcpy(value, idx+1, len);
			value[len] = '\0';
		}

		if (strcmp(key, "dir_store") == 0) setConfigStore(value);
		else if (strcmp(key, "dir_restore") == 0) setConfigRestore(value);
		else if (strcmp(key, "dir_key") == 0) setConfigKey(value);
		else if (strcmp(key, "client_name") == 0) setConfigClientName(value);
		else if (strcmp(key, "server_port") == 0) setConfigServerPort(atoi(value));
		else {
			printf ( "ERROR: Invalid parameter.\n" );
			exit(EXIT_FAILURE);
		}
	}

	fclose(file);
}

void saveConfig() {
	if (file_config == NULL) {
		printf ( "ERROR: Config-file not set.\n" );
		exit(EXIT_FAILURE);
	}

	FILE *file;
	file = fopen(file_config, "w");
	if (file == NULL) {
		printf ( "ERROR: Unable to write config-file (%s).\n", file_config );
		exit(EXIT_FAILURE);
	}

	fprintf(file, "%s=%s\n", "dir_store", dir_store);
	fprintf(file, "%s=%s\n", "dir_restore", dir_restore);
	fprintf(file, "%s=%s\n", "dir_key", dir_key);
	fprintf(file, "%s=%s\n", "client_name", client_name);
	char buf[33];
	sprintf(buf, "%d", server_port);
	fprintf(file, "%s=%s\n", "server_port", buf);

	fclose(file);

}

char *getConfigClientName() {
	return client_name;
}

void setConfigClientName(char *value) {
	if (client_name != NULL) free(client_name); /* free memory */

	if (value == NULL) {
		char *hostname = getHostname();
		client_name = malloc (strlen(hostname)+1);
		strcpy(client_name, hostname);
	} else {
		client_name = malloc (strlen(value)+1);
		strcpy(client_name, value);
	}

	printf ( "CLIENT-NAME: %s\n", client_name );
}

char *getConfigHome() {
	return dir_config;
}

void setConfigStore(char *value) {
    if (dir_store != NULL) free(dir_store);     /* free memory */

	if (value == NULL) {
		dir_store = malloc (strlen(dir_config)+1+strlen("store")+1);
		sprintf(dir_store, "%s/store", dir_config);
	} else {
		dir_store = malloc (strlen(value)+1);
		strcpy(dir_store, value);
	}

	mkdir(dir_store, 0755);
	if (!directoryExists(dir_store)) {
		printf ( "ERROR: Directory (%s) doesn't exists.\n", dir_store );
		exit(EXIT_FAILURE);
	}

	printf ( "STORE: %s\n", dir_store );
}

char *getConfigStore() {
	return dir_store;
}

void setConfigRestore(char *value) {
    if (dir_restore != NULL) free(dir_restore); /* free memory */

	if (value == NULL) {
		dir_restore = malloc (strlen(dir_config)+1+strlen("restore")+1);
		sprintf(dir_restore, "%s/restore", dir_config);
	} else {
		dir_restore = malloc (strlen(value)+1);
		strcpy(dir_restore, value);
	}

	mkdir(dir_restore, 0755);
	if (!directoryExists(dir_restore)) {
		printf ( "ERROR: Directory (%s) doesn't exists.\n", dir_restore );
		exit(EXIT_FAILURE);
	}

	printf ( "RESTORE: %s\n", dir_restore );
}

char *getConfigRestore() {
	return dir_restore;
}

void setConfigKey(char *value) {
    if (dir_key != NULL) free(dir_key);         /* free memory */

	if (value == NULL) {
		dir_key = malloc (strlen(dir_config)+1+strlen("key")+1);
		sprintf(dir_key, "%s/key", dir_config);
	} else {
		dir_key = malloc (strlen(value)+1);
		strcpy(dir_key, value);
	}
	
	mkdir(dir_key, 0755);
	if (!directoryExists(dir_key)) {
		printf ( "ERROR: Directory (%s) doesn't exists.\n", dir_key );
		exit(EXIT_FAILURE);
	}
	printf ( "KEY: %s\n", dir_key );
}

char *getConfigKey() {
	return dir_key;
}

void setConfigServerPort(int value) {
	if (value  <= 0) {
		server_port = CONFIG_SERVER_PORT;
	} else {
		server_port = value;
	}

	printf ( "SERVER-PORT: %d\n", server_port );
}

int getConfigServerPort() {
	return server_port;
}
