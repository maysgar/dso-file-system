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
int checkSyncFS();
int testOutput(int ret, char * msg);

/**
 * Test all the funtionalities of the method mkFS
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int test_mkFS(){
	/* Normal execution of mkFS */
	if(testOutput(mkFS(DEV_SIZE), "mkFS") < 0) {return -1;}
	/* Check the correct assigned values of the superblock in the FS */
    if(testOutput(checkMakeFS(), "checkMakeFS") < 0) {return -1;}
    /* Check the correct writing of the superblock into the disk */
    if(testOutput(checkSyncFS(), "syncFS") < 0) {return -1;}

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

/**
 * Checks the correct writing of the superblock data into the disk
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkSyncFS(){
    char * buf = malloc(sizeof(char) * 1 * SIZE_OF_BLOCK); /* auxiliary buffer */
    /* read the first block of the disk */
    if( bread(DEVICE_IMAGE, 1, buf) < 0){
        printf("Error in bread (mountFS)\n");
        return -1;
    }
    char * aux = (char *) (&sb);
    char * buf2 = malloc(sizeof(char) * 1 * SIZE_OF_BLOCK); /* auxiliary buffer */
    for(int i = 0; i < SIZE_OF_BLOCK; i++){
        buf2[i] = aux[i];
    }
    buf2[SIZE_OF_BLOCK] = '\0';
    if(strcmp(buf, buf2) != 0){ return -1;} /* the first blocks are different */
    return 0;
}

/**
 * Print the output message of a test method
 *
 * @param ret: the return value of the method executed
 * @param msg: String to print which is the name of the method executed
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int testOutput(int ret, char * msg){
    if(ret != 0) {
        fprintf(stdout, "%s%s%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST " ,msg, " ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
        return -1;
    }
    fprintf(stdout, "%s%s%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST ", msg, " ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
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
