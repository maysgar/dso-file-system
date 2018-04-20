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

/* mkFS tests */
int test_mkFS();
int checkMakeFS();
int checkSyncFS();
int cmpDisk (int startPoint, int blocks, char * structToComp);

/* mountFS tests */
int test_mountFS();

/* unmountFS tests */
int test_unmountFS();
int checkUnmountFS();

int testOutput(int ret, char * msg);

/* createFile tests */
int test_createFile();
int checkCreateFile();

/* removeFile tests */
int test_removeFile();
int checkRemoveFile();



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

int test_createFile(){
	/* Normal execution of createFile */
	if(testOutput(createFile("test.txt"), "createFile") < 0) {return -1;}
	/* Check the correct assigned values of the superblock in the FS */
    if(testOutput(checkCreateFile(), "checkCreateFile") < 0) {return -1;}

    printf("\n");
	return 0;
}

int test_removeFile(){
	/* Normal execution of removeFile */
	if(testOutput(removeFile("test.txt"), "removeFile") < 0) {return -1;}
	/* Check the correct assigned values of the superblock in the FS */
    if(testOutput(checkRemoveFile(), "checkRemoveFile") < 0) {return -1;}

	printf("\n");
	return 0;
}

/**
 * Checks the correct creation of files inside "disk.data"
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkCreateFile(){
	if(strcmp(inode[0].name, "") == 0){
		return -1;
	}
	if(inode[0].size != 0){ /* check number of blocks for the inode map */
		return -1;
	}
	if(inode[0].directBlock != 0){ /* check number of blocks for the data map */
		return -1;
	}
	if(strcmp(inode[0].padding, (char *)malloc(SIZE_OF_BLOCK - (NAME_MAX+(sizeof(int)*2)))) != 0){ // check number of inodes 
		return -1;
	}
	if(sb.i_map[0] != 1){
		return -1;
	}
	return 0;
}	

/**
 * Checks the correct creation of files inside "disk.data"
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkRemoveFile(){
	if(strcmp(inode[0].name, "") != 0){
		return -1;
	}
	if(inode[0].size != 0){ /* check number of blocks for the inode map */
		return -1;
	}
	if(inode[0].directBlock == 44){ /* check number of blocks for the data map */
		return -1;
	}
	if(strcmp(inode[0].padding, "") != 0){ // check number of inodes 
		return -1;
	}
	if(sb.i_map[0] != 0){
		return -1;
	}
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
	if(sb.numInodes != INODE_MAX_NUMBER){ /* check number of inodes */
		return -1;
	}
	if(sb.dataBlockNum != needed_blocks(DEV_SIZE, 'B')){ /* check the number of data blocks */
		return -1;
	}
	if(sb.deviceSize != DEV_SIZE){ /* check the size of the File System */
		return -1;
	}
	if(sb.firstInode != ( 2  )){ /* check the correct position of the first inode */
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
    /* compare the superblock with the first block of the disk */
    if(cmpDisk(1, SIZE_OF_BLOCK, (char *) (&sb)) < 0){ return -1;}

	/* compare the inodes with the ones at the disk */
	for(int i = 0; i < (sb.numInodes * sizeof(inode_t) / BLOCK_SIZE) ; i++){
		if(cmpDisk(i + sb.firstInode, SIZE_OF_BLOCK , (char*) (&inode[i])) < 0){ return -1;}
	}

	return 0;
}

/**
 * Compares the blocks from the disk with the superblock
 *
 * @param startPoint: the starting block to read blocks
 * @param blocks: the number of blocks to compare
 * @param structToComp: is the struct to compare, such as the superblock, convert to an array of bytes
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int cmpDisk (int startPoint, int blocks, char * structToComp){
    char * deviceBuf = malloc(sizeof(char) * blocks); /* auxiliary buffer */
    /* read the first block of the disk */
    if( bread(DEVICE_IMAGE, startPoint, deviceBuf) < 0){
        printf("Error in bread (mountFS)\n");
        return -1;
    }
    char * checkBuf = malloc(sizeof(char) * blocks); /* auxiliary buffer */
    for(int i = 0; i < SIZE_OF_BLOCK; i++){
		checkBuf[i] = structToComp[i];
    }
	checkBuf[SIZE_OF_BLOCK] = '\0';
    if(strcmp(deviceBuf, checkBuf) != 0){ return -1;} /* the first blocks are different */
    return 0;
}

/**
 * Test all the funtionalities of the method mountFS
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int test_mountFS(){
	/* Normal execution of mountFS */
	if(testOutput(mountFS(), "mountFS") < 0) {return -1;}

	/* Check the correct writing of the disk into the superblock */
	if(testOutput(checkSyncFS(), "syncFS") < 0) {return -1;} /* we reuse the same test */

	printf("\n");
	return 0;
}

/**
 * Test all the funtionalities of the method unmountFS
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int test_unmountFS(){
	/* Normal execution of unmountFS */
	if(testOutput(unmountFS(), "unmountFS") < 0) {return -1;}

	/* Normal execution of unmountFS */
	if(testOutput(checkUnmountFS(), "checkUnmountFS") < 0) {return -1;}

	printf("\n");
	return 0;
}

/**
 * Checks that the file system has been correctly unmount from the simulated device
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkUnmountFS(){
	/* check if the inode map is empty */
	if(strcmp(sb.i_map, "") != 0){ return -1;}

	/* check if the inode map is empty */
	if(strcmp(sb.b_map, "") != 0){ return -1;}

    /* check if the inodes are empty */
    for(int i = 0; i < sb.numInodes; i++) { /* block bitmap */
        if(strcmp(inode[i].name, "") != 0){ return -1;}
    }
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

	/*** test for making the File System ***/
	test_mkFS();

	/*** test for mounting the File System ***/
	test_mountFS();

	/*** test for unmounting the File System ***/
	test_unmountFS();

	/*** test for creating a file ***/
	test_createFile();

	/*** test for removing a file ***/
	test_removeFile();

	ret = unmountFS();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	return 0;
}
