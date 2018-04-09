/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	01/03/2017
 */

#define DISKSIZE 51200 //50KiB to 10 MiB (25 blocks)
#define INODE_BITMAP_[25] //1 Byte per Block
#define BLOCK_BITMAP_[25] //1 Byte per Block

#define bitmap_getbit(bitmap_, i_) (bitmap_[i_ >> 3] & (1 << (i_ & 0x07)))
static inline void bitmap_setbit(char *bitmap_, int i_, int val_) {
  if (val_)
    bitmap_[(i_ >> 3)] |= (1 << (i_ & 0x07));
  else
    bitmap_[(i_ >> 3)] &= ~(1 << (i_ & 0x07));
}

typedef struct{
  unsigned int magicNum;            /*Magic number of the superblock*/
  unsigned int InodeMapNumBlocks;   /*Number of blocks of the i-node map*/
  unsigned int DataMapNumBlock;     /*Number of blocks of the data map*/
  unsigned int numinodes;           /*Number of i-nodes in the device*/
  unsigned int firstinode;          /*Number of the 1st i-node in the device*/
  unsigned int dataBlockNum;        /*Number of data blocks in the device*/
  unsigned int firstDataBlock;      /*Number of the 1st data block*/
  unsigned int deviceSize;          /*Total disk space*/
  char padding[992];                /*Padding field for fulfilling a block*/
} superblock_t;

typedef struct{
  unsigned int type;                 /*FILE_T*/
  char name[200];                    /*file name*/
  unsigned int inodesContent[200];   /*directory i-node list*/
  unsigned int size;                 /*Current file size in Bytes*/
  unsigned int directBlock;          /*Direct block Number*/
  char padding[PADDING_inode];       /*Padding field for fulfilling a block*/
} inode_t;
