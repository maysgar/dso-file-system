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
  unsigned int magicNum;                                    /*Magic number of the superblock*/
  //NF6 The file system will be used on disks from 50 KiB to 10 MiB.
  //Funcion para determinar esto, ya que es un rango de valores...
  unsigned int deviceSize;                                  /*Total disk space*/
  unsigned int InodeMapNumBlocks[40];                       /*Number of blocks of the i-node map*/
  unsigned int DataMapNumBlock[deviceSize/SIZE_OF_BLOCK];   /*Number of blocks of the data map*/
  //NF1 The maximum number of files in the file system will never be higher than 40.
  unsigned int numinodes = 40;                              /*Number of i-nodes in the device*/
  //DETERMINED IN create_disk.c IN FIRST ARGUMENT (atoi(argv[1]))
  unsigned int dataBlockNum = deviceSize/SIZE_OF_BLOCK;     /*Number of data blocks in the device*/
  char padding[];                                           /*Padding field for fulfilling a block*/
  //unsigned int firstinode;                                /*Number of the 1st i-node in the device*/
  //unsigned int firstDataBlock;                            /*Number of the 1st data block*/
} superblock_t;

typedef struct{
  //?????
  char type = 'REGULAR';            /*FILE_T*/
  //NF2 The maximum length of the file name will be 32 characters.
  char name[32];                    /*file name*/
  //NF3 The maximum size of the file will be 1 MiB.
  unsigned int size = 1048576;      /*Current file size in Bytes*/
  char padding[];                   /*Padding field for fulfilling a block*/
  //unsigned int inodesContent[200];  /*i-node list*/
} inode_t;

int indirect_block[SIZE_OF_BLOCK/POINTER_BYTES]; /*Indirect Block*/
//DEFINE POINTER_BYTES
