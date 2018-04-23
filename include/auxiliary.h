/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	auxiliary.h
 * @brief 	Headers for the auxiliary functions required by filesystem.c.
 * @date	01/03/2017
 */
int umount (void); /* write the default File System into the disk */
int syncFS(void); /* writes the metadata into the disk */

int needed_blocks(int bits, char type);
int blocks_toWrite();
int getInodePosition(char *fileName);
int ialloc (void);
int alloc (void);
int ifree (int inode_id);
int bfree (int block_id);
int bmap(int inode_position, int offset);
int syncSP();
int syncIN();
int blocks_toWrite(int bytesToWrite, int fileSize, int blockSize);
