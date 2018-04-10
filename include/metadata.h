/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	01/03/2017
 */
 #define SIZE_OF_BLOCK 2048

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
  //NF1 The maximum number of files in the file system will never be higher than 40.
  unsigned int numinodes = 40;      /*Number of i-nodes in the device*/
  //unsigned int firstinode;          /*Number of the 1st i-node in the device*/
  unsigned int dataBlockNum;        /*Number of data blocks in the device*/
  //unsigned int firstDataBlock;      /*Number of the 1st data block*/
  unsigned int deviceSize;          /*Total disk space*/
  char padding[];                   /*Padding field for fulfilling a block*/
} superblock_t;

typedef struct{
  char type = 'REGULAR';            /*FILE_T*/
  //NF2 The maximum length of the file name will be 32 characters.
  char name[32];                    /*file name*/
  unsigned int inodesContent[200];  /*directory i-node list*/
  unsigned int size;                /*Current file size in Bytes*/
  char padding[];                   /*Padding field for fulfilling a block*/
} inode_t;

int indirect_block[SIZE_OF_BLOCK/POINTER_BYTES]; /*Indirect Block*/
//DEFINE POINTER_BYTES
