/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	01/03/2017
 */
#define SIZE_OF_BLOCK 1024 * 2      /* The file system block size will be 2048 bytes */
#define INODE_MAX_NUMBER 40         /* Maximum number of i-nodes in the device */
#define MAX_SIZE_FILE 1024 * 1024   /* Maximum file size in bytes */
#define NAME_MAX 32                 /* NF2 The maximum length of the file name will be 32 characters */
#define PADDING_SUPERBLOCK 999      /* Padding size for the superblock */
#define PADDING_INODE 999           /* Padding size for the inode */
#define MAX_BLOCK_PER_FILE 512      /* Maximum number of blocks per file */
#define IMAP_SIZE INODE_MAX_NUMBER / 8 /* Maximum number of imap entries */
#define BMAP_SIZE ( ((MAX_FILE_SYSTEM_SIZE) / (SIZE_OF_BLOCK)) -1)  / 8 /* Maximum number of bmap entries */


/* Variables used in mkFS for validating the size of the device */
#define MIN_FILE_SYSTEM_SIZE 50 * 1024    /* Minimum file system size */
#define MAX_FILE_SYSTEM_SIZE 10 * 1024 * 1024 /* Maximum file system size */

#define INODE_SIZE NAME_MAX + 8 /* Size of an inode in bytes */
#define INODE_PER_BLOCK (int) ( (SIZE_OF_BLOCK) / (INODE_SIZE)) /* Amount of inodes which fit in a block */

#define bitmap_getbit(bitmap_, i_) (bitmap_[i_ >> 3] & (1 << (i_ & 0x07)))
static inline void bitmap_setbit(char *bitmap_, int i_, int val_) {
  if (val_)
    bitmap_[(i_ >> 3)] |= (1 << (i_ & 0x07));
  else
    bitmap_[(i_ >> 3)] &= ~(1 << (i_ & 0x07));
}

typedef struct{
    unsigned int magicNum;              /* Magic number of the superblock */
    unsigned int numInodes;             /* Number of i-nodes in the device */
    unsigned int firstInode;            /* Number of the 1st i-node in the device */
    unsigned int dataBlockNum;          /* Number of data blocks in the device */
    unsigned int firstDataBlock;        /* Number of the 1st data block */
    unsigned int deviceSize;            /* Total disk space in bytes */
    unsigned int inodesBlocks;          /* Number of blocks for the inodes */
    char i_map [IMAP_SIZE];             /* inode map */
    char b_map [BMAP_SIZE];             /* block map */
    char padding[PADDING_SUPERBLOCK];   /* Padding field for fulfilling a block */
} superblock_t;

typedef struct{
    char name[NAME_MAX];                /* file name */
    unsigned int size;                  /* Current file size in Bytes */
    unsigned int directBlock;           /* Direct block number */
    unsigned int ptr;                   /* Pointer on the file */
} inode_t;

typedef struct{
    inode_t inodeArray [INODE_PER_BLOCK]; /* Inode array */
    char padding[PADDING_INODE];        /* Padding field for fulfilling a block */
} inode_block_t;


void printSuperBlock(superblock_t superBlock);
int umount (void); /* write the default File System into the disk */
int syncFS(void); /* writes the metadata into the disk */

int needed_blocks(int bits, char type);
void printInode(inode_t inode);
int getInodePosition(char *fileName);
int ialloc (void);
int alloc (void);
int ifree (int inode_id);
int bfree (int block_id);
int bmap(int inode_position, int offset);
