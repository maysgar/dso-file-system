/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	01/03/2017
 */
#define SIZE_OF_BLOCK 2048          /* The file system block size will be 2048 bytes */
#define INODE_MAX_NUMBER 40         /* Maximum number of i-nodes in the device */
#define MAX_SIZE_FILE 1048576       /* Maximum file size in bytes */
#define NAME_MAX 32                 /* NF2 The maximum length of the file name will be 32 characters */
#define MAX_DIR_CAPACITY 200        /* Por poner algo */
#define PADDING_SUPERBLOCK 999      /* Padding size for the superblock */
#define PADDING_INODE 999           /* Padding size for the inode */

/* Variables used in mkFS for validating the size of the device */
#define MIN_FILE_SYSTEM_SIZE 51200    /* Minimum file system size */
#define MAX_FILE_SYSTEM_SIZE 10485760 /* Maximum file system size */

#define TYPE_FILE 'F'
#define TYPE_DIRECTORY 'D'

#define bitmap_getbit(bitmap_, i_) (bitmap_[i_ >> 3] & (1 << (i_ & 0x07)))
static inline void bitmap_setbit(char *bitmap_, int i_, int val_) {
  if (val_)
    bitmap_[(i_ >> 3)] |= (1 << (i_ & 0x07));
  else
    bitmap_[(i_ >> 3)] &= ~(1 << (i_ & 0x07));
}

typedef struct{
  unsigned int magicNum;              /* Magic number of the superblock */
  unsigned int inodeMapNumBlocks;     /*Number of blocks of the i-node map*/
  unsigned int dataMapNumBlock;       /* Number of blocks of the data map */
  unsigned int numInodes;             /* Number of i-nodes in the device */
  unsigned int firstInode;            /* Number of the 1st i-node in the device */
  unsigned int dataBlockNum;          /* Number of data blocks in the device */
  unsigned int firstDataBlock;        /* Number of the 1st data block */
  unsigned int deviceSize;            /* Total disk space in bytes */
  char padding[PADDING_SUPERBLOCK];   /* Padding field for fulfilling a block */
} superblock_t;

typedef struct{
  char type;                          /* File or Directory */
  char name[NAME_MAX];                /* file name */
  unsigned int inodesContent[MAX_DIR_CAPACITY]; /* Directory inode list */
  unsigned int size;                  /* Current file size in Bytes */
  unsigned int directBlock;           /* Direct block number */
  char padding[PADDING_INODE];        /* Padding field for fulfilling a block */
} inode_t;

char * i_map;                         /* inode map */
char * b_map;                         /* block map */


// int indirect_block[SIZE_OF_BLOCK/POINTER_BYTES]; /*Indirect Block*/ Gabolo me lo vas a explicar tu
//DEFINE POINTER_BYTES
