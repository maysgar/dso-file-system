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
int checkMaxName();
int checkCreateAgain();
int checkMaxFiles();

/* removeFile tests */
int test_removeFile();
int checkRemoveFile();
int checkWrongRemove();

/* openFile tests */
int test_openFile();
int checkMaxNameOpen();

/* closeFile tests */
int test_closeFile();
int checkCloseFile();

/* lseek tests */
int test_lseek();
int checkBigLseek();
int checkWhenceLseek();
int checkNegativeLseek();
int checkNoFileLseek();


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
 * Test all the funtionalities of the method createFile
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int test_createFile(){
	/* Normal execution of createFile */
	if(testOutput(createFile("test.txt"), "createFile") < 0) {return -1;}
	/* Check the correct assigned values of the superblock in the FS */
    if(testOutput(checkCreateFile(), "checkCreateFile") < 0) {return -1;}
	/* Check the correct error handling when the length of the file name is bigger than the maximum */
	if(testOutput(checkMaxName(), "checkMaxName") < 0) {return -1;}
	/* Check the correct error handling when the file to be created already exists */
	if(testOutput(checkCreateAgain(), "checkCreateAgain") < 0) {return -1;}
	/* Check the correct error handling when the maximum number of files in the FS is achieved */
	if(testOutput(checkMaxFiles(), "checkMaxFiles") < 0) {return -1;}
    printf("\n");
	return 0;
}

/**
 * Test all the funtionalities of the method removeFile
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int test_removeFile(){
	createFile("test.txt");
	/* Normal execution of removeFile */
	if(testOutput(removeFile("test.txt"), "removeFile") < 0) {return -1;}
	/* Check the correct assigned values of the superblock in the FS */
    if(testOutput(checkRemoveFile(), "checkRemoveFile") < 0) {return -1;}
	/* Check the correct error handling when the file to be removed does not exist*/
	if(testOutput(checkWrongRemove(), "checkWrongRemove") < 0) {return -1;}

	printf("\n");
	return 0;
}

/**
 * Checks the correct creation of files inside "disk.data"
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkCreateFile(){
	if(strcmp(inodeList[0].inodeArray[0].name, "") == 0){
		return -1;
	}
	if(inodeList[0].inodeArray[0].size != 0){ /* check number of blocks for the inode map */
		return -1;
	}
	if(inodeList[0].inodeArray[0].directBlock != 0){ /* check number of blocks for the data map */
		return -1;
	}
	if(inodeList -> inodeArray[0].opened != 0){ /* check file created is closed */
		return -1;
	}
	/*if(strcmp(inode[0].padding, (char *)malloc(SIZE_OF_BLOCK - (NAME_MAX+(sizeof(int)*2)))) != 0){ // check number of inodes     Cambiar padding
		return -1;
	}*/
	if(sb.i_map[0] != 1){
		return -1;
	}
	return 0;
}

/**
 * Checks the correct error handling when the name of the file to be created is bigger thant the maximum
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkMaxName(){
	if(createFile("This string is bigger than the allowed maximum size for a file name") != -2) {
		return -1; /* createFile does not  check this case */
	}
	return 0;
}

/**
 * Checks the correct error handling when the when the file to be created already exists
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkCreateAgain(){
	if( (createFile("test.txt")) != -1){
		return -1; /* createFile does not  check this case */
	}
	return 0;
}

/**
 * Checks the correct error handling when the maximum number of files in the FS is achieved
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkMaxFiles(){
	/* start with a new FS */
	if(unmountFS() < 0){
		return -1; /* Error in the unmount */
	}
	for(int i = 0; i < INODE_MAX_NUMBER; i++){
		char aux = i + '0';
		char name [10];
		strcpy(name, &aux);
		if(createFile(name) < 0) { /* create all the files */
			return -1; /* error before arriving to the maximum number of files */
		}
	}
	/* Trying to create a file when there is no more space for it */
	if(createFile("Wrong file") >= 0){
		return -1; /* file created */
	}
	unmountFS(); /* unmount the FS  */
	return 0;
}

/**
 * Checks the correct creation of files inside "disk.data"
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkRemoveFile(){
	if(strcmp(inodeList[0].inodeArray[0].name, "") != 0){
		return -1;
	}
	if(inodeList[0].inodeArray[0].size != 0){ /* check number of blocks for the inode map */
		return -1;
	}
	if(inodeList[0].inodeArray[0].directBlock == 44){ /* check number of blocks for the data map */
		return -1;
	}
	if(inodeList[0].inodeArray[0].opened != 0){ /* check number of blocks for the data map */
		return -1;
	}
	if(sb.i_map[0] != 0){
		return -1;
	}
	return 0;
}

/**
 *  Check the correct error handling when the file to be removed does not exist
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkWrongRemove(){
	if(removeFile("Wrong File") >= 0){
		return -1;
	}
	return 0;
}

/**
 * Test all the funtionalities of the method openFile
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int test_openFile(){
	/* Normal execution of openFile */
	if(testOutput(openFile("test.txt"), "openFile") < 0) {return -1;}
	/* Check the correct opened values of the superblock in the FS */
    if(testOutput(checkMaxNameOpen(), "checkMaxNameOpen") < 0) {return -1;}

    printf("\n");
	return 0;
}

/**
 * Checks the correct error handling when the name of the file to be opened is bigger thant the maximum
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkMaxNameOpen(){
	/* Check file name is not larger than 32 characters */
	if(openFile("This string is bigger than the allowed maximum size for a file name") >= 0){
		return -1;
	}
	return 0;
}

/**
 * Test all the funtionalities of the method closeFile
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int test_closeFile(){
	/* Normal execution of closeFile */
	if(testOutput(closeFile(0), "closeFile") < 0) {return -1;}
	/* Check the correct assigned values of the superblock in the FS */
    if(testOutput(checkCloseFile(), "checkCloseFile") < 0) {return -1;}

    printf("\n");
	return 0;
}

/**
 * Checks the selected file is closed correctly
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkCloseFile(){

	if(0 < 0){ /* check the input file descriptor is negative */
		return -1;
	}
	if(inodeList[0].inodeArray[0].opened != 0){ /* check the file is closed */
		return -1;
	}
	
	return 0;
}


/**
 * Test all the funtionalities of the method lseek
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int test_lseek(){
	/* Normal execution of lseek */
	inodeList[0].inodeArray[0].size = 20;
	if(testOutput(lseekFile(0, -5, FS_SEEK_BEGIN), "lseek") < 0) {return -1;}
	if(testOutput(checkBigLseek(), "checkBigLseek") < 0) {return -1;}
	if(testOutput(checkNegativeLseek(), "checkNegativeLseek") < 0) {return -1;}
	if(testOutput(checkWhenceLseek(), "checkWhenceLseek") < 0) {return -1;}
	if(testOutput(checkNoFileLseek(), "checkNoFileLseek") < 0) {return -1;}


    printf("\n");
	return 0;
}

/**
 * Test case where the file pointer is moved more than than the end of the file
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkBigLseek(){
	if(lseek(0, 25, FS_SEEK_BEGIN) > 0){
		return -1;
	}
	return 0;
	
}

/**
 * Test case where the whence has a wrong value
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkWhenceLseek(){
	if(lseek(0, 2, 4) > 0){
		return -1;
	}
	return 0;
}

/**
 * Test case where the file pointer is moved before than the beginning of the file
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkNegativeLseek(){
	if(lseek(0, -25, FS_SEEK_CUR) > 0){
		return -1;
	}
	return 0;
}

/**
 * Test case where the file pointer does not correspond to an existing file
 *
 * @return 0 if all the tests are correct and -1 otherwise
 */
int checkNoFileLseek(){
	if(lseek(2, 2, FS_SEEK_CUR) > 0){
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
	if(sb.firstInode != ( 2 )){ /* check the correct position of the first inode */
		return -1;
	}
	if(sb.inodesBlocks != (int) ((INODE_MAX_NUMBER / INODE_PER_BLOCK)+1)){
		return -1;
	}
	if(sb.firstDataBlock != ( sb.firstInode + sb.inodesBlocks )){ /* check the correct position of the first data block */
		return -1;
	}
	for(int i = 0; i < sb.numInodes; i++){
		if(sb.i_map[i] != 0 || sb.b_map[i] != 0){
			return -1;
		}
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
	for(int i = 0; i < sb.inodesBlocks; i++){
		if(cmpDisk(i + sb.firstInode, SIZE_OF_BLOCK , (char*) (&inodeList)) < 0){ return -1;}
	}

	return 0;
}

/**
 * Compares the blocks from the disk with the superblock
 *
 * @param startPoint: the starting block to read blocks
 * @param blocks: the number of bytes to compare
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

	int count = 0;

	/* check if the inodes are empty */
	for(int i = 0; i < sb.inodesBlocks; i++){ /* check all the blocks of inodes */
		inode_block_t inodeListAux = inodeList[i]; /* copy the list of inodes of the current block */
		for(int j = 0; j <= INODE_PER_BLOCK; j++){ /* go through all the inodes from a block */
			if(count > INODE_MAX_NUMBER){ /* already checked all the inodes */
				return 0;
			}
			if(strcmp(inodeListAux.inodeArray[j].name, "") != 0){ return -1;}
			count++;
		}
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

	/* create the file to continue testing */
	createFile("test.txt");

	/*** test for opening a file ***/
	test_openFile();

	unmountFS();
	createFile("text.txt");
	openFile("text.txt");

	/*** test for moving the pointer of a file ***/
	test_lseek();

	/*** test for closing a file ***/
	test_closeFile();

	return 0;
}
