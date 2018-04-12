/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	filesystem.c
 * @brief 	Implementation of the core file system funcionalities and auxiliary functions.
 * @date	01/03/2017
 */

#include <stdlib.h>
#include <string.h>

#include "include/filesystem.h"		// Headers for the core functionality
#include "include/auxiliary.h"		// Headers for auxiliary functions
#include "include/metadata.h"		// Type and structure declaration of the file system
#include "include/crc.h"			// Headers for the CRC functionality


superblock_t sb; /* superblock */
inode_t * inode; /* array of inodes */


/*
 * @brief 	Generates the proper file system structure in a storage device, as designed by the student.
 *
 * NF4 The file system block size will be 2048 bytes.
 * NF6 The file system will be used on disks from 50 KiB to 10 MiB.
 * NF7 The size of on-disk filesystem metadata shall be minimized. (mierda)
 * NF8 The implementation shall not waste available resources. (mierda)
 *
 * @param deviceSize: size of the disk to be formatted in bytes.
 * @return 	0 if success, -1 otherwise.
 */
int mkFS(long deviceSize)
{
	int deviceSizeInt = (int) (deviceSize); /* convert the size of the file to integer */
	/* check the validity of the size of the device */
	if(deviceSizeInt < MIN_FILE_SYSTEM_SIZE || deviceSizeInt > MAX_FILE_SYSTEM_SIZE){
		return -1;
	}
	/* Superblock's magic number */
	sb.magicNum = 1; /* por poner algo */
	/* Number of blocks of the inode map */
	sb.inodeMapNumBlocks = INODE_MAX_NUMBER; /* as many entries as inodes */
	/* Number of blocks of the data map */
	sb.dataMapNumBlock = deviceSizeInt / SIZE_OF_BLOCK; /* The size of the device over the block size */
	/* Number of inodes in the device */
	sb.numInodes = INODE_MAX_NUMBER; /* Stated in the PDF */
	/* Number of the first inode */
	//sb.firstInode = -1; /* There is no inode */
	sb.firstInode = 0; /* * */
	/* Number of data blocks in the device */
	sb.dataBlockNum = deviceSizeInt / SIZE_OF_BLOCK; /* The size of the device over the block size */
	/* Number of the first data block */
	//sb.firstDataBlock = -1; /* Initially there is no data */
	sb.firstDataBlock = 0; /* * */
	/* Set the size of the disk */
	sb.deviceSize = deviceSizeInt;

	/* memory for the inodes */
	inode = malloc(sizeof(inode_t) * sb.numInodes);

	/* allocating memory to the bitmaps */
	i_map = malloc(sizeof(char) * sb.numInodes); /* inode bitmap */
	b_map = malloc(sizeof(char) * sb.dataBlockNum); /* block bitmap */

	/* Setting as free all the bitmap positions */
	for(int i = 0; i < sb.numInodes; i++){ /* inode bitmap */
		i_map[i] = 0; /* free */
	}
	for(int i = 0; i < sb.dataBlockNum; i++){ /* block bitmap */
		b_map[i] = 0; /* free */
	}

	/* Free the inodes */
	for(int i = 0; i < sb.numInodes; i++){ /* block bitmap */
		memset(&(inode[i]), 0, sizeof(inode_t));
	}

	printSuperBlock(sb);
	return 0;
}

/*
 * @brief 	Mounts a file system in the simulated device.
 *
 * NF5 Metadata shall persist between unmount and mount operations.
 *
 * @return 	0 if success, -1 otherwise.
 */
int mountFS(void)
{
	// bread();  El gabolo y sus liaditas
	return -1;
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 *
 * NF5 Metadata shall persist between unmount and mount operations.
 *
 * @return 	0 if success, -1 otherwise.
 */
int unmountFS(void)
{
	return -1;
}

/*
 * @brief	Creates a new file, provided it it doesn't exist in the file system.
 *
 * NF1 The maximum number of files in the file system will never be higher than 40.
 * NF2 The maximum length of the file name will be 32 characters.
 * NF3 The maximum size of the file will be 1 MiB.
 *
 * @param fileName: name of the file to be created.
 * @return	0 if success, -1 if the file already exists, -2 in case of error.
 */
int createFile(char *fileName)
{
	return -2;
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @param fileName: name of the file to be removed.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
	return -2;
}

/*
 * @brief	Opens an existing file and initializes its seek pointer to the beginning of the file.
 *
 * F2 Every time a file is opened, its seek pointer will be reset to the beginning of the file.
 * F5 File integrity must be checked, at least, on open operations.
 *
 * @param fileName: name of the file to be opened.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
 */
int openFile(char *fileName)
{
	return -2;
}

/*
 * @brief	Closes a file.
 * @param fileName: name of the file to be created.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
	return -1;
}

/*
 * @brief	Reads a number of bytes from a file, starting from the seek pointer of the file, and stores them in a buffer.
 * The seek pointer of the file is incremented as many bytes read from the file.
 *
 * F6 The whole contents of a file could be read by means of several read operations.
 *
 * @param fileDescriptor: file descriptor of the file to be read.
 * @param buffer: buffer that will store the read data after the execution of the function.
 * @param numBytes: number of bytes to read from the file.
 *
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fileDescriptor, void *buffer, int numBytes)
{
	return -1;
}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * The seek pointer of the file is incremented as many bytes written from the file.
 *
 * In case the operation exceeds the number of data blocks initially reserved for the file,
 * new data blocks shall be reserved without violating the filesystem limits.
 *
 * F3 Metadata shall be updated after any write operation in order to properly reflect any modification in the file system.
 * F7 A file could be modified by means of write operations
 * F8 As part of a write operation, file capacity may be extended by means of additional data blocks.
 * NF3 The maximum size of the file will be 1 MiB.
 *
 * @param fileDescriptor: file descriptor of the file to write into.
 * @param buffer: data to be written.
 * @param numBytes: number of bytes to write to the file from the buffer.
 *
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
	return -1;
}


/*
 * @brief	Modifies the position of the seek pointer of a file according to a given reference and offset.
 *
 * @param fileDescriptor: file descriptor.
 * @param whence: Constant value acting as reference for the seek operation. This could be: FS SEEK CUR, FS SEEK BEGIN or FSSEEKEND.
 * @param offset: Number of bytes to displace the seek pointer from the FS SEEK CUR position.
 * This value can be either positive or negative, although it will never allow positioning the seek pointer outside the limits of the file.
 * If whence is set to FS SEEK BEGIN or FS SEEK END, the file pointer must be set to this position, regardless of the offset value.
 *
 * @return	0 if success, -1 otherwise.
 */
int lseekFile(int fileDescriptor, long offset, int whence)
{
	return -1;
}

/*
 * @brief 	Verifies the integrity of the file system metadata.
 * @return 	0 if the file system is correct, -1 if the file system is corrupted, -2 in case of error.
 */
int checkFS(void)
{
	return -2;
}

/*
 * @brief 	Verifies the integrity of a file.
 * @return 	0 if the file is correct, -1 if the file is corrupted, -2 in case of error.
 */
int checkFile(char *fileName)
{
	return -2;
}

void printSuperBlock(superblock_t superBlock){
	if(printf("Magic number: %d\n", superBlock.magicNum) < 0){
		printf("Could not print Magic number");
	}
	if(printf("Number of blocks of the i-node map: %d\n", superBlock.inodeMapNumBlocks) < 0){
		printf("Could not print Number of blocks of the i-node map");
	}
	if(printf("Number of blocks of the data map: %d\n", superBlock.dataMapNumBlock) < 0){
		printf("Could not print Number of blocks of the data map");
	}
	if(printf("Number of i-nodes in the device: %d\n", superBlock.numInodes) < 0){
		printf("Could not print Number of i-nodes in the device");
	}
	if(printf("Number of the 1st i-node in the device  %d\n", superBlock.firstInode) < 0){
		printf("Could not print Number of the 1st i-node in the device");
	}
	if(printf("Number of data blocks in the device: %d\n", superBlock.dataBlockNum) < 0){
		printf("Could not print Number of data blocks in the device");
	}
	if(printf("Number of the 1st data block: %d\n", superBlock.firstDataBlock) < 0){
		printf("Could not print Number of the 1st data block");
	}
	if(printf("Total disk space: %d\n", superBlock.deviceSize) < 0){
		printf("Could not print Total disk space");
	}
	if(printf("Padding field: %s\n", superBlock.padding) < 0){
		printf("Could not print Padding field:");
	}
}