#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_DIRECTORY ".SimpleShare"
#define CONFIG_FILE "config"

#define CONFIG_SERVER_PORT 1234

void initConfig();
void releaseConfig();
char *getConfigHome();
void setConfigStore(char *value);
char *getConfigStore();
void setConfigRestore(char *value);
char *getConfigRestore();
void setConfigKey(char *value);
char *getConfigKey();
void setConfigClientName(char *value);
char *getConfigClientName();
void setConfigServerPort(int value);
int getConfigServerPort();

#endif


