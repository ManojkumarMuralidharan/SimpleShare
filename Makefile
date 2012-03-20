CFLAGS += -m64
CFLAGS += -Wall -I . -I lib/md5-1.3.0 -I /data/devel/Laukien/lib/la-C -g
#CFLAGS +=-O3
LIB += lib/common.o lib/config.o lib/error.o lib/md5-1.3.0/md5.o
LA = /data/devel/Laukien/lib/la-C/la_64.a


all: library test compile

library:
	@echo LIBRARY
	$(CC) $(CFLAGS) -c -o lib/common.o src/common.c
	$(CC) $(CFLAGS) -c -o lib/config.o src/config.c
	$(CC) $(CFLAGS) -c -o lib/error.o src/error.c
	$(CC) $(CFLAGS) -c -o lib/address.o src/address.c

test:
	@echo TEST
	$(CC) $(CFLAGS) -o bin/md5 src/md5.c $(LIB) $(LA)

compile:
	@echo COMPILE
	$(CC) $(CFLAGS) -o bin/SimpleShareStore src/store.c $(LIB) $(LA)
	$(CC) $(CFLAGS) -o bin/SimpleShareCheck src/check.c $(LIB) $(LA)
	$(CC) $(CFLAGS) -o bin/SimpleShareRestore src/restore.c $(LIB) $(LA)
	$(CC) $(CFLAGS) -o bin/SimpleShareHost src/host.c lib/address.o $(LIB) $(LA)
	$(CC) $(CFLAGS) -o bin/SimpleShareServer src/server.c lib/address.o $(LIB) $(LA)
	$(CC) $(CFLAGS) -o bin/SimpleShareDownload src/download.c lib/address.o $(LIB) $(LA)
