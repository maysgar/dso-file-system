/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	test.c
 * @brief 	Implementation of the client test routines.
 * @date	01/03/2017
 */

#include <stdio.h>
#include <string.h>
#include "include/filesystem.h"
#include "filesystem.c"


// Color definitions for asserts
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE   "\x1b[34m"

#define N_BLOCKS	25						// Number of blocks in the device
#define DEV_SIZE 	N_BLOCKS * BLOCK_SIZE	// Device size, in bytes

int checkMakeFS();

/**
 * Test all the funtionalities of the method mkFS
 */
int test_mkFS(){
	int ret; /* return variable */

	/* Normal execution of mkFS */
	ret = mkFS(DEV_SIZE);
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mkFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mkFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	/* Check the correct assigned values of the superblock in the FS */
	ret = checkMakeFS();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST checkMakeFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST checkMakeFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	printf("\n");
	return 0;
}

/**
 * Checks the correct assigning of values to the superblock of the FS
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkMakeFS(){
	if(sb.magicNum != 1){ /* check magic number */
		return -1;
	}
	if(sb.inodeMapNumBlocks != 1){ /* check number of blocks for the inode map */
		return -1;
	}
	if(sb.dataMapNumBlock != ( BITMAP_BLOCK / 1024 )){ /* check number of blocks for the data map */
		return -1;
	}
	if(sb.numInodes != INODE_MAX_NUMBER){ /* check number of inodes */
		return -1;
	}
	if(sb.dataBlockNum != ( DEV_SIZE / SIZE_OF_BLOCK )){ /* check the number of data blocks */
		return -1;
	}
	if(sb.deviceSize != DEV_SIZE){ /* check the size of the File System */
		return -1;
	}
	if(sb.firstInode != ( 2 + sb.inodeMapNumBlocks + sb.dataMapNumBlock )){ /* check the correct position of the first inode */
		return -1;
	}
	if(sb.firstDataBlock != ( sb.firstInode + sb.numInodes )){ /* check the correct position of the first data block */
		return -1;
	}
	return 0;
}

int main() {
	int ret; /* return variable */

	/*** test for make the File System ***/
	test_mkFS();


	ret = mountFS();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	ret = createFile("test.txt");
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	ret = unmountFS();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	return 0;
}
