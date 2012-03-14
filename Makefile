CFLAGS += -m64
CFLAGS += -Wall -I lib/md5-1.3.0 -g
#CFLAGS +=-O3
LIB += lib/common.o lib/config.o lib/error.o lib/md5-1.3.0/md5.o


all: library test compile

library:
	$(CC) $(CFLAGS) -c -o lib/common.o src/common.c
	$(CC) $(CFLAGS) -c -o lib/config.o src/config.c
	$(CC) $(CFLAGS) -c -o lib/error.o src/error.c
	$(CC) $(CFLAGS) -c -o lib/address.o src/address.c

test:
	$(CC) $(CFLAGS) -o bin/md5 src/md5.c $(LIB)

compile:
	$(CC) $(CFLAGS) -o bin/SimpleShareStore src/store.c $(LIB)
	$(CC) $(CFLAGS) -o bin/SimpleShareCheck src/check.c $(LIB)
	$(CC) $(CFLAGS) -o bin/SimpleShareRestore src/restore.c $(LIB)
	$(CC) $(CFLAGS) -o bin/SimpleShareHost src/host.c $(LIB) lib/address.o
	$(CC) $(CFLAGS) -o bin/SimpleShareServer src/server.c $(LIB) lib/address.o
	$(CC) $(CFLAGS) -o bin/SimpleShareDownload src/download.c $(LIB) lib/address.o
