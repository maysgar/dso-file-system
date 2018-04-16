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
	/* Number of data blocks in the device */
	sb.dataBlockNum = needed_blocks(deviceSizeInt, 'B'); /* The size of the device over the block size */
	/* Number of blocks of the data map */
	sb.dataMapNumBlock = needed_blocks(sb.dataBlockNum, 'b'); /* as many as the maximum amount of files */
	/* Number of inodes in the device */
	sb.numInodes = INODE_MAX_NUMBER; /* Stated in the PDF */
	/* Number of blocks of the inode map */
	sb.inodeMapNumBlocks = needed_blocks(sb.numInodes,'b'); /* as many bits as inodes */
	/* Set the size of the disk */
	sb.deviceSize = deviceSizeInt;
    /* Number of the first inode */
    sb.firstInode = 2 + sb.inodeMapNumBlocks + sb.dataMapNumBlock; /* the first inode is after the data clock bitmap */
    /* Number of the first data block */
    sb.firstDataBlock = sb.firstInode + sb.numInodes; /* after the last inode */

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

	/* write the default file system into disk */
	if( umount() < 0 ){ /* check for errors in umount */
		printf("Error in umount\n");
		return -1;
	}
	printSuperBlock(sb);
	printInode(inode[0]);
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
	/*
	free(inode);
	free(i_map);
	free(b_map);
	return 0;
	*/
    // free function does not have a return value 

	/* delete inode map */
    memset(&(i_map), 0, sb.inodeMapNumBlocks);
	/* delete block map */
	memset(&(b_map), 0, sb.dataMapNumBlock);
	/* Free the inodes */
	for(int i = 0; i < sb.numInodes; i++){ /* block bitmap */
		memset(&(inode[i]), 0, sizeof(inode_t));
	}
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
	/* Check NF2 */
	if(strlen(fileName) > NAME_MAX){
		printf("File name too long. The maximum length for a file name is 32 characters\n");
		return -2;
	}
	/*
	off_t size = 0;
	if((size = lseek(openFile(fileName),0,SEEK_END)) < 0){
		printf("Could not move the pointer inside the file\n");
		return -2;
	}
	if(size > MAX_SIZE_FILE){
		printf("File size too long. The maximum length for a file name is 32 characters\n");
		printf("Size: %d\n", (int)size);
		return -2;
	}
	*/

	/*
	struct stat st;
	stat(fileName, &st);
	// Check NF3 
	if(st.st_size > MAX_SIZE_FILE){
		printf("File too large. The maximum size for a file is 1 MiB\n");
		printf("Size: %d\n", (int)st.st_size);
		return -2;
	}
	*/
	int i = 0;
	while(strcmp(inode[i].name, "") != 0){
		if(fileName == inode[i].name){  //OJO CUIDADO strcmp() !!!!!!!
			printf("File already exists\n");
			return -1;
		}
		i++;
	}
	int position = 0;
    i = 0;
	while(i_map[i] == 1 && i < INODE_MAX_NUMBER){
		i++;
	}
	if(i_map[i] == 1){
		printf("Maximum number of files reached (40)\n");
		return -2;
	}
	if(i_map[i] == 0){
		position = i;
	} /* free */

    /*
	OTHER OLDER APPROACH
	for(int i = 0; i < INODE_MAX_NUMBER; i++){ // inode bitmap 
		if(i_map[i] == 0){
			position = 1;
		} // free
	}
	// Check NF1 
	if(position == 0){
		printf("Maximum number of files reached (40)\n");
		return -2;
	}
	*/
	strcpy(inode[position].name, fileName);
	/* inode[position].size = st.st_size; */
	//inode[position].size = (int)size;
	inode[position].size = 0;
	int j = 0;
	while(b_map[j] == 1){
		j++;
	}
	//mirar si el bitmap de bloques está lleno
	i = 0;
	int positionB = 0;
	while(b_map[i] == 1 && i < INODE_MAX_NUMBER){
		i++;
	}
	if(b_map[i] == 1){
		printf("Maximum number of files reached (40)\n");
		return -2;
	}
	if(i_map[i] == 0 && i == position){
		positionB = i;
	} /* free */
	/* Set the position of the new file as full in the bmap */
	b_map[positionB] = 1;
	inode[position].directBlock = j;
	//inode[position].padding = SIZE_OF_BLOCK - (32+(4*2);  // unisgned int 2 or 4 bytes????? meterle mierda a pincho hasta que ocupe todo (0) //poner las constantes !!!!!!!!!!!

	/* Set the position of the new file as full in the imap */
	i_map[position] = 1;
	return 0;
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @param fileName: name of the file to be removed.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)													//DEBERIAMOS TAMBIEN CERRAR EL FILE?? closeFile(fileDes);
{
	for(int i = 0; i < sb.numInodes; i++){
		if(fileName == inode[i].name){   ///OJO CUIDADO STRSCMP()
			//inode[i].name = "/0";		 //????
			strcpy(inode[i].name, "/0");        //MIRAR MEMSET()
			//strcpy(inode[i].name, "");        //otra opcion
			inode[i].size = 0;           //????
			inode[i].directBlock = 0;    //????
			//inode[i].padding = "/0";     //????
			strcpy(inode[i].padding, "/0");

			/* Set the position of the new file as free in the imap */
			i_map[i] = 0;
			//bitmap de bloques a 0 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			printf("File %s deleted\n", fileName);
			return 0;
		}
	}
	printf("File %s does not exist\n", fileName);
	return -1;
	//printf("Error\n");  
	//return -2;                        //Don't know when to return this
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
	//CRC MIRAR TEMITA INTEGRITY
	int fileDes = 0;
	for(int i = 0; i < sb.numInodes; i++){
		if(fileName == inode[i].name){
			//#include <fcntl.h> !!!!!!!!!!
			if((fileDes = open(fileName, O_RDWR)) < 0){   //PELEAS MUY MUY FUERTES CON EL PATH  /filesystem/fileName
				printf("Error opening file: %s\n", fileName);
				return -2;
			}
			if(lseek(fileDes, 0, SEEK_SET) < 0){
				printf("Error moving the pointer of file: %s\n", fileName);
				return -2;
			}
			printf("File %s successfully opened\n", fileName);
			return fileDes;
		}
	}
	printf("File %s does not exist\n", fileName);
	return -1;
}

/*
 * @brief	Closes a file.
 * @param fileDescriptor: descriptor of the file to close.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
	//MIRAR PDF
	if(close(fileDescriptor) < 0){
		printf("Error closing the file\n");
		return -1;
	}
	printf("File closed successfully\n");
	return 0;
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
	ssize_t bytesRead = 0;
	while(bytesRead < numBytes){
		if(bytesRead += read(fileDescriptor, buffer, numBytes) < 0){ //read files 1 byte by one
		printf("Error reading the file\n");
		return -1;
		}
	}
	printf("%zd bytes successfully read from the file\n", bytesRead);
	return (int)bytesRead;
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
	ssize_t bytesWritten = 0;
	while(bytesWritten < numBytes){
		if(bytesWritten += write(fileDescriptor, buffer, numBytes) < 0){
			printf("Error when writing in the file\n");
			return -1;
		}
	}
	printf("%zd bytes successfully written in the file\n", bytesWritten);
	return (int)bytesWritten;
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
	if(lseek(fileDescriptor, offset, whence) < 0){
		printf("Error moving the pointer of file: %d\n", fileDescriptor);
		return -1;
	}
	printf("Seek pointer successfully moved\n");
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
		aux = (amount/8)/SIZE_OF_BLOCK;
		if(((amount/8)%SIZE_OF_BLOCK) != 0){
			aux++;
		}
	}
	else if(type == 'B'){
		aux = amount/SIZE_OF_BLOCK;
		if((amount%SIZE_OF_BLOCK) != 0){
			aux++;
		}
	}
	else{
		printf("Wrong input type.\n bits: 'b'\n Bytes: 'B'\n");
		return -1;
	}
	printf("Blocks needed to store %d bits/bytes is: %d\n", amount, aux);
	return aux;
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
