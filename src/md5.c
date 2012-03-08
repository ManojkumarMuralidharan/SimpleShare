/*
 * =====================================================================================
 *
 *       Filename:  md5.c
 *
 *    Description:  ShareDAV - MD5
 *
 *        Version:  1.0
 *        Created:  01/24/2012 08:59:02 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Stephan Laukien (Ditha), software@laukien.com
 *        Company:  
 *
 * =====================================================================================
 */

#include "common.h"
#include "md5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char sig[MD5_SIZE];
char buffer[BLOCK_SIZE];
md5_t md5;
int ret;

int main(int argc, char *argv[]) {
	showCopyright("Split");

	/* check arguments */
	if (argc != 2) {
		printf ( "ERROR: Invalid arguments.\n" );
		printf ( "\t%s STRING\n", argv[0] );
		exit(EXIT_FAILURE);
	}

	char *string = argv[1];
	printf ( "String:\t%s\n", string );
	printf ( "MD5:\t" );
	md5_buffer(string, strlen(string), sig);
	const unsigned char *sig_p;
	for (sig_p = sig; sig_p < sig + MD5_SIZE; sig_p++) {
		printf("%02x", *sig_p);
	}
	printf ( "\n" );

	return(0);
}
