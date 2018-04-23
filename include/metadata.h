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
#define MAX_BLOCK_PER_FILE 512      /* Maximum number of blocks per file */
#define IMAP_SIZE (INODE_MAX_NUMBER / 8) /* Maximum number of imap entries */
#define BMAP_SIZE ( (((MAX_FILE_SYSTEM_SIZE) / (SIZE_OF_BLOCK)) -1)  / 8) /* Maximum number of bmap entries */

/* Variables used in mkFS for validating the size of the device */
#define MIN_FILE_SYSTEM_SIZE 50 * 1024    /* Minimum file system size */
#define MAX_FILE_SYSTEM_SIZE 10 * 1024 * 1024 /* Maximum file system size */

#define bitmap_getbit(bitmap_, i_) (bitmap_[i_ >> 3] & (1 << (i_ & 0x07)))
static inline void bitmap_setbit(char *bitmap_, int i_, int val_) {
  if (val_)
    bitmap_[(i_ >> 3)] |= (1 << (i_ & 0x07));
  else
    bitmap_[(i_ >> 3)] &= ~(1 << (i_ & 0x07));
}

/**  Unsigned Short range: 0 to 65,535
 *   Unsigned Int range: 0 to 4,294,967,295
 */

/*
 * Size of superblock_t:
 * shorts: 5
 * Ints: 2
 * Chars: IMAP_SIZE + BMAP_SIZE
 */
#define SUPERBLOCK_SIZE (5 * 2) + (2 * 4) + (IMAP_SIZE) + (BMAP_SIZE)
#define SUPERBLOCK_PADDING (SIZE_OF_BLOCK) - (SUPERBLOCK_SIZE) /* Padding size for the superblock */

typedef struct{
    unsigned short magicNum;              /* Magic number of the superblock */
    unsigned short numInodes;             /* Number of i-nodes in the device */
    unsigned short firstInode;            /* Number of the 1st i-node in the device */
    unsigned int dataBlockNum;            /* Number of data blocks in the device */
    unsigned short firstDataBlock;        /* Number of the 1st data block */
    unsigned int deviceSize;              /* Total disk space in bytes */
    unsigned short inodesBlocks;          /* Number of blocks for the inodes */
    char i_map [IMAP_SIZE];               /* inode map */
    char b_map [BMAP_SIZE];               /* block map */
    char padding[SUPERBLOCK_PADDING];     /* Padding field for fulfilling a block */
} superblock_t;

/*
 * Size of inode_t:
 * shorts: 2
 * Ints: 2
 * Chars: NAME_MAX
 */
  #define INODE_SIZE (2 * 2) + (2 * 4) + (NAME_MAX)  /* Size of an inode in bytes */

typedef struct{
    char name[NAME_MAX];                /* file name */
    unsigned int size;                  /* Current file size in Bytes */
    unsigned int indirectBlock;         /* Indirect block number */
    unsigned short opened;              /* To know if a file is opened or closed */
    unsigned short ptr;
} inode_t;

typedef struct{
    unsigned int pos[512];                /* Max file indexes of blocks */
} index_file_t;

/*
 * Size of inode_block_t:
 * INODE_PER_BLOCK * INODE_SIZE
 */
#define INODE_PER_BLOCK (int) ( (SIZE_OF_BLOCK) / (INODE_SIZE)) /* Amount of inodes which fit in a block */
#define INODE_BLOCK_SIZE (INODE_PER_BLOCK) * (INODE_SIZE) /* Size of inode_block_t */
#define INODE_BLOCK_PADDING (SIZE_OF_BLOCK) - (INODE_BLOCK_SIZE) /* Padding size for the inode_block_t */

typedef struct{
    inode_t inodeArray [INODE_PER_BLOCK]; /* Inode array */
    char padding[INODE_BLOCK_PADDING];    /* Padding field for fulfilling a block */
} inode_block_t;
