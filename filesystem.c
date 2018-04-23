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
inode_block_t * inodeList; /* Struct of inodes */

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
	/* Number of inodes in the device */
	sb.numInodes = INODE_MAX_NUMBER; /* Stated in the PDF */
	/* Set the size of the disk */
	sb.deviceSize = deviceSizeInt;
	/* Number of the first inode */
	sb.firstInode = 2; /* the first inode is after the superblock */

	/* calculate the number of inode_block_t that we need */
	sb.inodesBlocks = (int) (INODE_MAX_NUMBER / INODE_PER_BLOCK);
	if(((INODE_MAX_NUMBER % INODE_PER_BLOCK)) != 0){
		sb.inodesBlocks++;
	}

    /* Number of the first data block */
    sb.firstDataBlock = sb.firstInode + sb.inodesBlocks; /* after the last inode block */

	/* memory for the list of inodes */
	inodeList = malloc(sizeof(inode_block_t) * sb.inodesBlocks);

	/* Setting as free all the bitmap positions */
	for(int i = 0; i < sb.numInodes; i++){ /* inode bitmap */
		bitmap_setbit(sb.i_map, i, 0); /* free */
	}
	for(int i = 0; i < sb.dataBlockNum; i++){ /* block bitmap */
		bitmap_setbit(sb.b_map, i, 0); /* free */
	}

	/* Free the inode blocks */
	for(int i = 0; i < sb.inodesBlocks; i++){
		memset(&(inodeList[i]), 0, sizeof(inode_block_t));
	}

	/* write the default file system into disk */
	if( umount() < 0 ){ /* check for errors in umount */
		return -1;
	}
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
        return -1;
    }
    /* read the inodeList from disk */
    for(int i = 0; i < sb.inodesBlocks; i++){
        if( bread(DEVICE_IMAGE, i+sb.firstInode, (char *) (&inodeList) + i*BLOCK_SIZE) < 0){
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
	/* Free the inode blocks */
	for(int i = 0; i < sb.inodesBlocks; i++){
		memset(&(inodeList[i]), 0, sizeof(inode_block_t));
  	}
  	/* Free  */
  	for(int i = 0; i < sb.numInodes; i++){
		ifree(i);
		bfree(i);
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
	if(strlen(fileName) > NAME_MAX) return -2;

  	if(getInodePosition(fileName) >= 0) return -1;

	int position = ialloc(); /* get the position of a free inode */
    if(position < 0) {return -1;} /* error while ialloc */

	int bPos = alloc(); /* get the position of a free data block */
	if(bPos < 0) {return -1;} /* error while alloc */

	/* know in what block of inodes it is */
	int aux = position / INODE_PER_BLOCK;
	position = position % INODE_PER_BLOCK;

	inodeList[aux].inodeArray[position].indirectBlock = bPos;
	inodeList[aux].inodeArray[position].ptr = 0;

  	strcpy(inodeList[aux].inodeArray[position].name, fileName);
	inodeList[aux].inodeArray[position].size = 0;
	/* We set the new file to closed */
	inodeList[aux].inodeArray[position].opened = 0;

	syncFS();
	return 0;
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @param fileName: name of the file to be removed.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
	/* Name is too long */
	if(strlen(fileName) > NAME_MAX){
		return -2;
	}
	/* get the position of the file to be deleted */
	int position = getInodePosition(fileName);
	/* know in what block of inodes it is */
	int aux = position / INODE_PER_BLOCK;
	position = position % INODE_PER_BLOCK;

	if(position >= 0){
 		if(inodeList[aux].inodeArray[position].opened == 1){
			closeFile(position);
		}
		strcpy(inodeList[aux].inodeArray[position].name, "");
		inodeList[aux].inodeArray[position].size = 0;
		inodeList[aux].inodeArray[position].indirectBlock = 0;

		inodeList[aux].inodeArray[position].ptr = 0;

		bitmap_setbit(sb.i_map, position, 0);
		syncFS();
		return 0;
	}
	else{
		return -1;
	}
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
	int position = getInodePosition(fileName);
	/* check if the file exists */
	if(position < 0){ return -1;}
	 /* know in what block of inodes it is */
	 int aux = position / INODE_PER_BLOCK;
	 /* position inside the block */
	 int bPosition = position % INODE_PER_BLOCK;

	/* If the file is already opened */
	if(inodeList[aux].inodeArray[bPosition].opened == 1){
		return -1;
	}

	/* If the file name is the same as the one in the inode and the entry of
	that inode in the bitmap is not empty then the file is ready to be openned */
	if((strcmp(fileName,inodeList[aux].inodeArray[bPosition].name) == 0) && bitmap_getbit(sb.i_map,position) == 1){
		inodeList[aux].inodeArray[bPosition].opened = 1;
		/* Set pointer of file to 0 */
		if(inodeList[aux].inodeArray[bPosition].ptr > 0) inodeList[aux].inodeArray[bPosition].ptr = 0;
		syncIN();
		return position; //i is the file descriptor
	}
 	return -1;
 }

/*
 * @brief	Closes a file.
 * @param fileDescriptor: descriptor of the file to close.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
	//PDF: when the file descriptor is closed, all file blocks are flushed to disk
	if(fileDescriptor < 0){
		return -1;
	}

	/* know in what block of inodes it is */
	int aux = fileDescriptor / INODE_PER_BLOCK;
	/* position inside the block */
	int bPosition = fileDescriptor % INODE_PER_BLOCK;

	/* If the file is already closed */
	if(inodeList[aux].inodeArray[bPosition].opened == 0){
		return -1;
	}

	inodeList[aux].inodeArray[bPosition].opened = 0;
	syncIN();
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
  int bytesRead = 0;
  int aux = fileDescriptor / INODE_PER_BLOCK;
  int position = fileDescriptor % INODE_PER_BLOCK;
  int pointer = inodeList[aux].inodeArray[position].ptr;
  index_file_t indBlock;
  int needed_blocks = 0;

	 /* If the file descriptor does not exist or no bytes to read or pointer is set_pointer
       to the end of the file or if the inode is unused, error */
  if(fileDescriptor < 0 || fileDescriptor > sb.numInodes || numBytes == 0 || sb.i_map[fileDescriptor] == 0){
    return -1;
  }

  /* If the file is not opened we proceed to open it */
  if(inodeList[aux].inodeArray[position].opened == 0){
    openFile(inodeList[aux].inodeArray[position].name);
  }

  /* Read the indirect block of the inode */
  if(bread(DEVICE_IMAGE, inodeList[aux].inodeArray[position].indirectBlock, (char *)(&indBlock)) < 0){ return -1;}

  /* Retrieve inode of the file (fileDescriptor == index on array of inodes) */
  if(inodeList[aux].inodeArray[position].size == 0){ return 0;} /* Return 0 bytes (empty file) */
  /* Size is not equal to zero */
	
  lseekFile(fileDescriptor, 0, FS_SEEK_BEGIN);

  /* If the number of bytes to be read plus the bytes to be read are less than the size
  of the file then proceed to read as normal until the bytes to read have been read. */
  needed_blocks = blocks_toWrite(numBytes, inodeList[aux].inodeArray[position].size, BLOCK_SIZE);
  if(pointer + numBytes <= inodeList[aux].inodeArray[position].size){
      /* Read the inode until the numBytes has been read*/
	  for(int j = 0; j <= needed_blocks; j++) {
		  if (bread(DEVICE_IMAGE, sb.firstDataBlock + indBlock.pos[j], buffer+BLOCK_SIZE*j) < 0) { return -1; }
	  }
	  char newBuf[numBytes];
	  memcpy(newBuf,buffer,  numBytes);
	  newBuf[numBytes] = '\0';

	  memset(buffer,0, numBytes + 1);

      memcpy(buffer,newBuf,  numBytes);

      pointer += numBytes; /* Update pointer */
      syncIN();
      return bytesRead;
  }
  else{
      for(int j = 0; j <= needed_blocks; j++) {
          if (bread(DEVICE_IMAGE, sb.firstDataBlock + indBlock.pos[j], buffer+BLOCK_SIZE*j) < 0) { return -1; }
      }
      pointer = inodeList[aux].inodeArray[position].size;
      bytesRead = inodeList[aux].inodeArray[position].size-pointer;
      syncIN();
      return bytesRead;
    }
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
	int aux = fileDescriptor / (INODE_PER_BLOCK);
	int position = fileDescriptor % (INODE_PER_BLOCK);
	int needed_blocks = 0;
	int block_free = 0;
	index_file_t dummy;
	char buffer_w[BLOCK_SIZE];

	/* Errors... */
	if(fileDescriptor < 0 || fileDescriptor > sb.numInodes || numBytes <= 0
		 || sb.i_map[fileDescriptor] == 0){
  	  return -1;
  	}

	/* NF3 */
	if( (inodeList[aux].inodeArray[position].size + numBytes) > MAX_FILE_SIZE) return -1;

	/* If the file is not opened we proceed to open it */
	if(inodeList[aux].inodeArray[position].opened == 0){
	  openFile(inodeList[aux].inodeArray[position].name);
	}

	/* Calculate the number of blocks needed to write */
	needed_blocks = blocks_toWrite(numBytes, inodeList[aux].inodeArray[position].size, BLOCK_SIZE);
	
	block_free = alloc();

	for (int i = 0; i <= needed_blocks; i++) {
		dummy.pos[i] = alloc();
		bwrite(DEVICE_IMAGE, sb.firstDataBlock + dummy.pos[i], buffer+BLOCK_SIZE*i);
		bitmap_setbit(sb.b_map, dummy.pos[i], 1);
	}

	memcpy(buffer_w, &dummy, BLOCK_SIZE);
  	/* Update the size of the file and the pointer */
	inodeList[aux].inodeArray[position].size += numBytes;
	inodeList[aux].inodeArray[position].ptr += numBytes;

	/* Write the indirectBlock in disk */
	bwrite(DEVICE_IMAGE,block_free,buffer_w); /* store indirect in disk */
	inodeList[aux].inodeArray[position].indirectBlock = block_free;
	syncFS();
	return numBytes;
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
	/* know in what block of inodes it is */
	int aux = fileDescriptor / INODE_PER_BLOCK;
	/* position inside the block */
	int bPosition = fileDescriptor % INODE_PER_BLOCK;

	/* If the file is closed we cannot move its pointer */
	if(inodeList[aux].inodeArray[bPosition].opened == 0){
		return -1;
	}

	/* If the file descriptor does not exist or no bytes to read or the offset is larger than the file size */
	if(fileDescriptor < 0 || fileDescriptor > sb.numInodes || abs(offset) > inodeList[aux].inodeArray[bPosition].size){
		return -1;
	}

	/* Modify the position from the current one */
	if(whence == FS_SEEK_CUR){
		if((inodeList[aux].inodeArray[bPosition].ptr + offset) > inodeList[aux].inodeArray[bPosition].size){
			return -1;
		}
		if((inodeList[aux].inodeArray[bPosition].ptr + offset) < 0){
			return -1;
		}
		inodeList[aux].inodeArray[bPosition].ptr += offset;
	}
	/* Modify the position from the beginning of the file */
	else if(whence == FS_SEEK_BEGIN){
		inodeList[aux].inodeArray[bPosition].ptr = 0;
	}
	/* Modify the position from the end of the file */
	else if(whence == FS_SEEK_END){
		inodeList[aux].inodeArray[bPosition].ptr = inodeList[aux].inodeArray[bPosition].size;
	}
	else{
		/* The whence has a wrong value */
		return -1;
	}
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
		if((bitmap_getbit(sb.i_map, i)) == 1){ /* check if the inode is in used */
			return -1; /* inode in used  */
		}
	}

	/* flush metadata on disk */
	if( syncFS() < 0){ /* check errors in sync */
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
	if(syncSP() < 0){ return -1;}
	/* write the inode list to disk */
	if(syncIN() < 0) { return -1;}
	return 0;
}

/**
 * Writes the superblock into the disk
 *
 * @return -1 in error and 0 otherwise
 */
int syncSP(){
	/* write the superblock into the first block of the disk */
	if( bwrite(DEVICE_IMAGE, 1, (char *) (&sb)) < 0){
		return -1;
	}
	return 0;
}

/**
 * Writes the inode_block_t into the disk
 *
 * @return -1 in error and 0 otherwise
 */
int syncIN(){
	for(int i = 0; i < sb.inodesBlocks; i++){
		if( bwrite(DEVICE_IMAGE, i+sb.firstInode, (char *) (&inodeList) + i*BLOCK_SIZE) < 0){
			return -1;
		}
	}
	return 0;
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
		if(bitmap_getbit(sb.i_map, i) == 0){ /* check if the position is free */
			bitmap_setbit(sb.i_map, i, 1); /* inode busy */
			/* know in what block of inodes it is */
			int position = i;
			int aux = position / INODE_PER_BLOCK;
			position = position % INODE_PER_BLOCK;
            memset(&(inodeList[aux].inodeArray[position]), 0, sizeof(inode_t) ); /* default values to the inode */
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
        if(bitmap_getbit(sb.b_map, i) == 0){ /* check if the position is free */
			bitmap_setbit(sb.b_map, i, 1); /* block busy */
            memset(b, 0, BLOCK_SIZE); /* default values to the block */
            bwrite(DEVICE_IMAGE, i + sb.firstDataBlock, b); /* write the empty block in the position found */
            return (i + sb.firstDataBlock); /* return the position of the block */
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
	bitmap_setbit(sb.i_map, inode_id, 0);
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
	bitmap_setbit(sb.b_map, block_id, 0);
	return 0;
}

/**
 * Get the position of the given inode
 *
 * @param inode : the inode to extract the position
 * @return -1 in case of error an the position of the inode otherwise
 */
int getInodePosition(char *fname){
	int count = 0;
	for(int i = 0; i < sb.inodesBlocks; i++){ /* go through all the inode blocks */
		inode_block_t inodeListAux = inodeList[i]; /* copy the list of inodes of the current block */
		for(int j = 0; j <= INODE_PER_BLOCK; j++){ /* go through all the inodes from a block */
			if(count > INODE_MAX_NUMBER){ /* already checked all the inodes */
				return -1;
			}
			else if(strcmp(inodeListAux.inodeArray[j].name, fname) == 0) {
				/* Return file position */
				return i;
			}
			count++;
		}
	}
	return -1;
}

/**
 * Get the direct block from the inode in the given position
 *
 * @param inode_position : the position of the inode
 * @return -1 in case of error an the direct block of the inode otherwise
 */
int bmap(int inode_position, int offset){
	/* position is not valid */
	if(inode_position > sb.numInodes){
		return -1;
	}

	/* return the inode block */
	if(offset < SIZE_OF_BLOCK){
		/* know in what block of inodes it is */
		int aux = inode_position / INODE_PER_BLOCK;
		inode_position = inode_position % INODE_PER_BLOCK;
		return inodeList[aux].inodeArray[inode_position].indirectBlock;
	}
	return -1;
}

/**
 * Gets the number of blocks needed to write the requested bytes
 *
 * @param bytesToWrite: number of bytes requested to write, fileSize: size of the file, blockSize: size of the block
 * @return -1 in case of error, or the number of blocks needed otherwise
 *
 */
 int blocks_toWrite(int bytesToWrite, int fileSize, int blockSize){
	int remainder = fileSize % blockSize;
	int falta = blockSize - remainder;
	int newBlockBytes = bytesToWrite - falta;
	if(newBlockBytes < 0){
		return 0;
	}

	int newBlocks = newBlockBytes/blockSize;
	if(newBlockBytes % blockSize != 0){
		newBlocks++;
	}
	return newBlocks;
 }
