/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	filesystem.c
 * @brief 	Implementation of the core file system funcionalities and auxiliary functions.
 * @date	01/03/2017
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include "include/filesystem.h"		// Headers for the core functionality
#include "include/auxiliary.h"		// Headers for auxiliary functions
#include "include/metadata.h"		// Type and structure declaration of the file system
#include "include/crc.h"			// Headers for the CRC functionality
#include "blocks_cache.h"

superblock_t sb; /* superblock */
inode_t * inode; /* array of inodes */
char buffer_block[BLOCK_SIZE];

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
	if(deviceSizeInt < MIN_FILE_SYSTEM_SIZE || deviceSizeInt > MAX_FILE_SYSTEM_SIZE) return -1;

 	/* Superblock's magic number */
	sb.magicNum = 1; /* por poner algo */

 	/* Number of data blocks in the device */
 	sb.dataBlockNum = needed_blocks(deviceSizeInt, 'B'); /* The size of the device over the block size */

 	/* Number of inodes in the device */
 	sb.numInodes = INODE_MAX_NUMBER; /* Stated in the PDF */

 	/* Set the size of the disk */
 	sb.deviceSize = deviceSizeInt;

  /* Number of the first inode */
  sb.firstInode = 2; /* the first inode is after the superblock */

 	/* calculate the number of inode_block_t that we need */
 	sb.inodesBlocks = (int) (INODE_MAX_NUMBER / INODE_PER_BLOCK);
 	if(((INODE_MAX_NUMBER % INODE_PER_BLOCK)) != 0) sb.inodesBlocks++;

  /* Number of the first data block */
  sb.firstDataBlock = sb.firstInode + sb.inodesBlocks; /* after the last inode block */

	/* memory for the inodes */
 	inode = malloc(sizeof(inode_t) * sb.numInodes);

 	/* memory for the list of inodes */
 	inodeList = malloc(sizeof(inode_block_t) * sb.inodesBlocks);

 	/* Setting as free all the bitmap positions */
 	for(int i = 0; i < sb.numInodes; i++){ /* inode bitmap */
 		bitmap_setbit(sb.i_map, i, 0); /* free */
 	}

 	for(int i = 0; i < sb.dataBlockNum; i++){ /* block bitmap */
 		bitmap_setbit(sb.b_map, i, 0); /* free */
 	}

	 /* Free the inodes */
	 for(int i = 0; i < sb.numInodes; i++){
	 	memset(&(inode[i]), 0, sizeof(inode_t));
	 }

	 /* write the default file system into disk */
	 if( umount() < 0 )	return -1;

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
    /* read the superblock from the disk to the new superblock */
    if(bread(DEVICE_IMAGE, 1, (char *) (&sb)) < 0){
        printf("Error in bread (mountFS)\n");
        return -1;
    }
    /* read from disk inode map */
    for(int i = 0; i < sb.inodeMapNumBlocks; i++){
        if( bread(DEVICE_IMAGE, 2+i, (char *) (i_map) + i*BLOCK_SIZE) < 0){
            printf("Error in bread (mountFS)\n");
            return -1;
        }
    }
    /* read from disk block map */
    for(int i = 0; i < sb.dataMapNumBlock; i++){
        if( bread(DEVICE_IMAGE, 2+i+sb.inodeMapNumBlocks, (char *) (b_map) + i*BLOCK_SIZE) < 0){
            printf("Error in bread (mountFS)\n");
            return -1;
        }
    }
    /* read inodes from disk */
    for(int i = 0; i < (sb.numInodes * sizeof(inode_t) / BLOCK_SIZE); i++){
        if( bread(DEVICE_IMAGE, i+sb.firstInode, (char *) (inode) + i*BLOCK_SIZE) < 0){
            printf("Error in bread (mountFS)\n");
            return -1;
        }
    }
	return 0;
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
	int i = 0;

	/* delete inode map */
  memset(&(i_map), 0, sb.inodeMapNumBlocks);
	while(i_map[i] == 0) i++;
	if(i_map[i] == 1) return -1; /* Not emptied correctly */

	/* delete block map */
	memset(&(b_map), 0, sb.dataMapNumBlock);
	while(b_map[i] == 0) i++;
	if(b_map[i] == 1) return -1; /* Not emptied correctly */

	/* Free the inodes */
	for(int i = 0; i < sb.numInodes; i++) memset(&(inode[i]), 0, sizeof(inode_t));

	//BWRITE

	return 0;
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
	int i = 0;
	int position = ialloc(); 				/* get the position of a free inode */
	int bPos = alloc(); 						/* get the position of a free data block */

	/* Check NF2 */
	if(strlen(fileName) > NAME_MAX){
		printf("File name too long. The maximum length for a file name is 32 characters\n");
		return -2;
	}

	/* File already exists */
	for(int i = 0; i < sb.numInodes; i++){
		if(strcmp(fileName,inode[i].name) == 0){
			printf("File already exists\n");
			return -1;
		}
	}

	if(position < 0 || bPos < 0) return -1; 		/* error while ialloc */

	strcpy(inode[i].name,fileName); 						/* file name */
	inode[i].size = 0;													/* Size set to 0 as file is initially empty */
	inode[i].directBlock = bPos;								/* Set direct block to be pointed */
	inode[i].ptr = 0;														/* Set pointer of the file to 0 */
	strcpy(inode[i].padding, (char *)malloc(BLOCK_SIZE - (NAME_MAX+(sizeof(int)*2))));

	/* Write on disk the inode created */
	if(bwrite(DEVICE_IMAGE,sb.firstInode,buffer_block) == -1) return -3;

	return 0;
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @param fileName: name of the file to be removed.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
	for(int i = 0; i < sb.numInodes; i++){
		/* File does not exist */
		if(strcmp(inode[i].name,fileName) == -1){
			return -1;
		}
		else{ 															/* File Exists */
			i_map[i] = 0;											/* Free Bitmap */
			ifree(i]);												/* Free inode */
			return 0;
		}
	}
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
	for(int i = 0; i < sb.numInodes; i++){
		/* If the file name is the same as the one in the inode and the entry of
		that inode in the bitmap is not empty then the file is ready to be openned */
		if((strcmp(fileName,inode[i].name) == 0) && i_map[i] == 1){
			/* Set pointer of file to 0 */
			if(node[i].ptr > 0) inode[i].ptr = 0;
			return i; //i is the file descriptor
		}
	}
	printf("File %s does not exist\n", fileName);
	return -1;

	//CRC MIRAR TEMITA INTEGRITY
	//#include <fcntl.h> !!!!!!!!!!
}

/*
 * @brief	Closes a file.
 * @param fileDescriptor: descriptor of the file to close.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
	for(int i = 0; i < sb.numInodes; i++){
		/* If the fd is out of bounds, not in the inode list... */
		if(fileDescriptor < 0 || fileDescriptor > sb.numInodes) return -1;
		if(fileDescriptor == i){

		}
	}
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
	int bytesRead = 0;
	int numBytesAux = numBytes;
	/* If the file descriptor does not exist or no bytes to read*/
	if(fileDescriptor < 0 || fileDescriptor > sb.numInodes || numBytes == 0) return -1;

	/* Retrieve inode of the file (fileDescriptor == index on array of inodes) */
	if(inode[fileDescriptor].size == 0) return bytesRead; /* Return 0 bytes (empty file) */
	else{ //Size is not equal to zero
		/* Repeat the bread operation until "numBytes go to zero" */
		while(numBytesAux != 0){
			/* Read the inode until the numBytes has been read*/
			bread(DEVICE_IMAGE,sb.firstInode+fileDescriptor,buffer_block);
			/* If the number of bytes to be read are bigger than the size of the file */
			if(numBytes <= inode[fileDescriptor].size){
				/* It was read the whole file or partially */
				bytesRead = numBytes;
				numBytesAux = numBytesAux - bytesRead;
			}
			//else ?
		}
		//TODO: guardar en buffer
	}
	return bytesRead;
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
	int bytesWritten = 0;
	int numBytesAux = numBytes;
	/* If the file descriptor does not exist or no bytes to write */
	if(fileDescriptor < 0 || fileDescriptor > sb.numInodes || numBytes == 0) return -1;


	return bytesWritten;
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

	return 0;
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

/**
 * Writes the default File System into the disk
 * @return -1 in error and 0 otherwise
 */
int umount (void){
	/* check that all the files are closed  */
	for(int i = 0; i < sb.numInodes; i++){
		if(i_map[i] == 1){ /* check if the inode is in used */
			return -1; /* inode in used  */
		}
	}

	/* flush metadata on disk */
	if( syncFS() < 0){ /* check errors in sync */
		printf("Error in sync\n");
		return -1;
	}
	return 0;
}

/**
 * Writes the metadata into the disk
 *
 * @return -1 in error and 0 otherwise
 */
int syncFS (void){
	/* write the superblock into the first block of the disk */
	if( bwrite(DEVICE_IMAGE, 1, (char *) (&sb)) < 0){
	    printf("Error in bwrite (syncFS)\n");
        return -1;
	}
    /* write inode map to disk */
	for(int i = 0; i < sb.inodeMapNumBlocks; i++){
        if( bwrite(DEVICE_IMAGE, 2+i, (char *) i_map + i*BLOCK_SIZE) < 0){
            printf("Error in bwrite (syncFS)\n");
            return -1;
        }
	}
    /* write block map to disk */
    for(int i = 0; i < sb.dataMapNumBlock; i++){
        if( bwrite(DEVICE_IMAGE, 2+i+sb.inodeMapNumBlocks, (char *) b_map + i*BLOCK_SIZE) < 0){
            printf("Error in bwrite (syncFS)\n");
            return -1;
        }
    }
	/* write inodes to disk */
	for(int i = 0; i < (sb.numInodes * sizeof(inode_t) / BLOCK_SIZE) ; i++){
		if( bwrite(DEVICE_IMAGE, i+sb.firstInode, (char *) inode + i*BLOCK_SIZE) < 0){
			printf("Error in bwrite (syncFS)\n");
            return -1;
        }
	}
	return 0;
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

/**
 * Gives you the needed blocks to store the input bits or bytes
 *
 * @return -1 in case of error and the number of needed blocks otherwise
 */
int needed_blocks(int amount, char type){
	int aux = 0;
	if(type == 'b'){
		aux = (amount/8)/BLOCK_SIZE;
		if(((amount/8)%BLOCK_SIZE) != 0){
			aux++;
		}
	}
	else if(type == 'B'){
		aux = amount/BLOCK_SIZE;
		if((amount%BLOCK_SIZE) != 0){
			aux++;
		}
	}
	else{
		printf("Wrong input type.\n bits: 'b'\n Bytes: 'B'\n");
		return -1;
	}
	return aux;
}

/**
 * Searches for a free position in the inode map
 *
 * @return 	the position of the free inode. In case of error -1 is returned
 */
int ialloc(void){
    for(int i = 0; i < sb.numInodes; i++){
        if(i_map[i] == 0){ /* check if the position is free */
            i_map[i] = 1; /* inode busy */
            memset(&(inode[i]), 0, sizeof(inode_t) ); /* default values to the inode */
            return i; /* return the position of the inode */
        }
    }
    return -1;
}

/**
 * Searches for a free position in the block map
 *
 * @return 	the position of the free block. In case of error -1 is returned
 */
int alloc(void){
    char b[BLOCK_SIZE];
    for(int i = 0; i < sb.dataBlockNum; i++){
        if(b_map[i] == 0){ /* check if the position is free */
            b_map[i] = 1; /* block busy */
            memset(b, 0, BLOCK_SIZE); /* default values to the block */
            bwrite(DEVICE_IMAGE, i + sb.firstDataBlock, b); /* write the empty block in the position found */
            return i; /* return the position of the block */
        }
    }
    return -1;
}

/**
 * Free a position of an inode
 *
 * @param inode_id : the position of the inode to be deleted
 * @return -1 in case of error an 0 otherwise
 */
int ifree (int inode_id){
	/* check the validity of the position of the inode */
	if(inode_id > sb.numInodes) { return -1;}
	/* free inode */
	i_map[inode_id] = 0;
	return 0;
}

/**
 * Free a position of a block
 *
 * @param block_id : the position of the block to be deleted
 * @return -1 in case of error an 0 otherwise
 */
int bfree (int block_id){
	/* check the validity of the position of the block */
	if(block_id > sb.dataBlockNum) { return -1;}
	/* free block */
	b_map[block_id] = 0;
	return 0;
}

void printInode(inode_t inode){
	if(printf("File name: %s\n", inode.name) < 0){
        printf("Could not print Magic number");
    }
    if(printf("File size in bytes: %d\n", inode.size) < 0){
        printf("Could not print the File size");
    }
    if(printf("Direct block number: %d\n", inode.directBlock) < 0){
        printf("Could not print Direct block number");
    }
    if(printf("Padding: %s\n", inode.padding) < 0){
        printf("Could not print the padding");
    }
}
