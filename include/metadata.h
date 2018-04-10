/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	01/03/2017
 */
#define SIZE_OF_BLOCK 2048 /* The file system block size will be 2048 bytes */
#define INODE_NUMBER 40 /* Number of i-nodes in the device */
#define NAME_MAX 32  /* NF2 The maximum length of the file name will be 32 characters */

/* Variables used in mkFS for validating the size of the device */
#define MINIMUM_FS_SIZE 0.0
#define MAX_FILE_SYSTEM_SIZE 9999999.0 /* Habrá que cambiarlo un dia de estos */

#define TYPE_REGULAR "REGULAR"

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
  long deviceSize;                                           /*Total disk space*/
  unsigned int inodeMapNumBlocks[40];                       /*Number of blocks of the i-node map*/
  unsigned int * dataMapNumBlock;                           /*Number of blocks of the data map*/
  //NF1 The maximum number of files in the file system will never be higher than 40.
  unsigned int numinodes;                              /*Number of i-nodes in the device*/
  //DETERMINED IN create_disk.c IN FIRST ARGUMENT (atoi(argv[1]))
  unsigned int dataBlockNum;                                /*Number of data blocks in the device*/
  char padding[];                                           /*Padding field for fulfilling a block*/
  //unsigned int firstinode;                                /*Number of the 1st i-node in the device*/
  //unsigned int firstDataBlock;                            /*Number of the 1st data block*/
} superblock_t;

typedef struct{
  //?????
  char * type;            /*FILE_T*/
  char name[NAME_MAX];                    /*file name*/
  //NF3 The maximum size of the file will be 1 MiB.
  unsigned int size;      /*Current file size in Bytes*/
  char padding[];                   /*Padding field for fulfilling a block*/
  //unsigned int inodesContent[200];  /*i-node list*/
} inode_t;

// int indirect_block[SIZE_OF_BLOCK/POINTER_BYTES]; /*Indirect Block*/ Gabolo me lo vas a explicar tu
//DEFINE POINTER_BYTES
