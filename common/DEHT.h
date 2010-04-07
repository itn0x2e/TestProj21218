/********************************************************************/
/* File DEHT - Disk Embedded Hash Table. An API you must implement  */
/* It supports varied sizes binary key to binary data - very generic*/
/* Has no necessary link to rainbow tables, passwords, etc.         */
/* Read theory of DEHT before this interface                        */
/********************************************************************/
#ifndef _DEHT_H_
#define _DEHT_H_

#include <stdio.h>
#include "types.h"

/********************************************************************/
/* type DEHT_DISK_PTR stands for "pointers" representation in DEHT  */
/* Data-type of long (argument of "fseek" function) represents an   */
/* offset in a file, which is "disk pointers" in our implementation */
/********************************************************************/
#define DEHT_DISK_PTR    long 



#define KEY_FILE_EXT (".key")
#define DATA_FILE_EXT (".data")

/*! This would have all been so much simpler using a struct, but the specification demanded that everything here be dynamic... !*/
#define KEY_FILE_RECORD_SIZE(ht) (ht->header.nBytesPerValidationKey + sizeof(DEHT_DISK_PTR))

/* each block is structured in the following manner:
 * uint_t blockUsedRecordsCount (0 <= blockUsedRecordsCount <= nPairsPerBlock)
 * nPairsPerBlock elements of KEY_FILE_BLOCK_SIZE (easier access provided by cast to KeyFilePair_t)
 * DEHT_DISK_PTR nextBlockPtr (0 == no next block)
 */
#define KEY_FILE_BLOCK_SIZE(ht) (sizeof(uint_t) + ht->header.nPairsPerBlock * KEY_FILE_RECORD_SIZE(ht) + sizeof(DEHT_DISK_PTR))

/*! Not for direct use. !*/
/* First element in the block is the used blocks count */
#define __RECORD_COUNT_PTR(blockPtr) ((uint_t *) (blockPtr))

#define GET_USED_RECORD_COUNT(blockPtr) ( *(__RECORD_COUNT_PTR(blockPtr)) )
#define SET_USED_RECORD_COUNT(blockPtr, newCount) \
	do { \
		*(__RECORD_COUNT_PTR(blockPtr)) = newCount; \
	} while (0)



#define GET_N_REC_PTR_IN_BLOCK(ht, blockPtr, n) ((KeyFilePair_t *) (((byte_t *) blockPtr) + sizeof(uint_t) + n * KEY_FILE_RECORD_SIZE(ht)))

/*! Not intended for direct use !*/
#define __NEXT_BLOCK_PTR(ht, blockPtr) ((DEHT_DISK_PTR *) (((byte_t *) blockPtr) + KEY_FILE_BLOCK_SIZE(ht) - sizeof(DEHT_DISK_PTR)))

#define GET_NEXT_BLOCK_PTR(ht, blockPtr) ( *(__NEXT_BLOCK_PTR(ht, blockPtr)) )
#define SET_NEXT_BLOCK_PTR(ht, blockPtr, nextBlockPtr) \
	do { \
		*(__NEXT_BLOCK_PTR(ht, blockPtr)) = nextBlockPtr; \
	} while (0)

#define KEY_FILE_BUCKET_POINTERS_SIZE(ht) (ht->header.numEntriesInHashTable * sizeof(DEHT_DISK_PTR))

typedef struct KeyFilePair_s {
	DEHT_DISK_PTR dataOffset;
	byte_t key[1];
} KeyFilePair_t;


/******************************************************************/
/* structure of "first level header" - basic preferences of a DEHT*/
/******************************************************************/
struct DEHTpreferences
{
    char sDictionaryName[16];  /*Name for identification, e.g. "MD5\0" */
    int numEntriesInHashTable; /*typically few millions*/
    int nPairsPerBlock;        /*typically few hundreds*/
    int nBytesPerValidationKey;/*length of key to be compared into, 
							    e.g. 8 means 64bit key for validation*/ 
	int numUnrelatedBytesSaved; /*for example 4000 if you save 1000 ineteger*/
	/*********************************************************/
	/*It is completely OK to add several members of your own */
	/*Just remember that this struct is saved "as is" to disk*/
	/*So no pointers should be written here                  */
	/*********************************************************/
};

/******************************************************************/
/* Kind of data-structure DEHT_STATUS that can be -1,0,1 as flags */
/* should have been enum, but you did not learn this, so #define  */
/******************************************************************/
#define DEHT_STATUS_SUCCESS        1
#define DEHT_STATUS_FAIL          -1
#define DEHT_STATUS_NOT_NEEDED     0

/****************************************************************************/
/* type definition of hashKeyIntoTableFunctionPtr:                          */
/* Definition of what is a data-structre hash-function (not the cryptic one)*/
/* These function take a key and output an index in pointer table           */
/* Note that these function operates on the original key (not condensed one)*/
/* These function shall never fail (i.e. never return -1 or so)             */
/****************************************************************************/
typedef int (*hashKeyIntoTableFunctionPtr)(const unsigned char *,int,int); 
/*Arguments are: */
/* const unsigned char *keyBuf, i.e. Binary buffer input*/ 
/* int keySizeof , i.e. in this project this is crypt output size, */
/*          but in real life this size may vary (e.g. string input)*/ 
/* int nTableSize, i.e. Output is 0 to (nTableSize-1) to fit table of pointers*/ 

/****************************************************************************/
/* type definition of hashKeyforEfficientComparisonFunctionPtr:             */
/* I is made to create a key signature (stored in DEHT) that distinguish    */
/* it from any other key in same bucket. Namely to correct false matches    */
/* caused by the hashKeyIntoTableFunctionPtr, thus must be independent of it*/
/* Note that these functions consider nBytesPerValidationKey as hard coded  */ 
/* E.g. stringTo32bit(very widely used) or cryptHashTo64bit(as in this proj)*/
/****************************************************************************/
typedef int (*hashKeyforEfficientComparisonFunctionPtr)(const unsigned char *,int, unsigned char *); 
/*Arguments are: */
/* const unsigned char *keyBuf, i.e. Binary buffer input*/ 
/* int keySizeof , i.e. in this project this is crypt output size, */
/*          but in real life this size may vary (e.g. string input)*/ 
/* unsigned char *validationKeyBuf, i.e. Output buffer, assuming allocated with nBytesPerValidationKey bytes*/ 

			
/****************************************************************************/
/* type definition of DEHT ! a struct containing all required to specify one*/
/****************************************************************************/
typedef struct /*This struct holds all needed during actual calls*/
{
    char sKeyfileName[80]; /*filename (as OS recognize) of .key */
    char sDatafileName[80];/*filename (as OS recognize) of .data */
    FILE *keyFP;           /*file pointer to the .key file as stdio recognize*/ 
    FILE *dataFP;
    struct DEHTpreferences header; 
    hashKeyIntoTableFunctionPtr hashFunc;                          /*key to table of pointers*/
	hashKeyforEfficientComparisonFunctionPtr comparisonHashFunc;   /*key to validation process (distinguish collision for real match*/
    DEHT_DISK_PTR *hashTableOfPointersImageInMemory;      /*null or some copy of what in file in case we cache it - efficient to cache this and header only*/
	DEHT_DISK_PTR *hashPointersForLastBlockImageInMemory; /*null or some intermidiate to know whenever insert. It has no parallel on disk*/
	int *anLastBlockSize; /*null or some intermidiate to know whenever insert. It has no parallel on disk. Block size to enable quick insert*/
	/***YOU ARE ALLOWED TO ADD HERE EXTRA MEMBERS, BUT BE EFFICIENT**/
	/***I don't think extra members are necessary                  **/
}DEHT;

/********************************************************************************/
/* Function create_empty_DEHT creates a new DEHT.                               */
/* Inputs: file names on disk (as prefix), hashing functions,                   */
/*    identification name, and parameters regarding memory management           */
/* Output:                                                                      */
/* If fail, Returns NULL and prints informative error to stderr)                */
/* It dump header by itself. Also null table of pointers.                       */
/* Notes:                                                                       */
/* Open them in RW permission (if exist then fail, do not overwrite).           */
/* hashTableOfPointersImageInMemory, hashPointersForLastBlockImageInMemory:=NULL*/
/********************************************************************************/
DEHT *create_empty_DEHT(const char *prefix,/*add .key and .data to open two files return NULL if fail creation*/
                        hashKeyIntoTableFunctionPtr hashfun, hashKeyforEfficientComparisonFunctionPtr validfun,
                        const char *dictName,   /*e.g. MD5\0 */ 
                        int numEntriesInHashTable, int nPairsPerBlock, int nBytesPerKey); /*optimization preferences*/

/********************************************************************************/
/* Function load_DEHT_from_files importes files created by previously used DEHT */
/* Inputs: file names on disk (as prefix).                                      */
/* Output: an allocated DEHT struct pointer.                                    */
/* Notes:                                                                       */
/* It open files (RW permissions) and create appropriate data-strucre on memory */
/* hashTableOfPointersImageInMemory, hashPointersForLastBlockImageInMemory:=NULL*/
/* Returns NULL if fail (e.g. files are not exist) with message to stderr       */
/********************************************************************************/
DEHT *load_DEHT_from_files(const char *prefix,
						   hashKeyIntoTableFunctionPtr hashfun, hashKeyforEfficientComparisonFunctionPtr validfun); 


/* utility functions for c'tors & d'tor */
DEHT * DEHT_init_instance (const char * prefix, char * keyFileMode, char * dataFileMode, 
			   hashKeyIntoTableFunctionPtr hashfun, hashKeyforEfficientComparisonFunctionPtr validfun);
void DEHT_freeResources(DEHT * instance, bool_t removeFiles);

/********************************************************************************/
/* Function insert_uniquely_DEHT inserts an ellement.                           */
/* Inputs: DEHT to insert into, key and data (as binary buffer with size)       */
/* Output: just status of action:                                               */
/* If exist updates data and returns DEHT_STATUS_NOT_NEEDED                     */
/* If successfully insert returns DEHT_STATUS_SUCCESS.                          */
/* If fail, returns DEHT_STATUS_FAIL                                            */
/* Notes:                                                                       */
/* if hashTableOfPointersImageInMemory use it                                   */
/* if  null, do not load table of pointers into memory just make simple         */
/* insert using several fseek when necessary.                                   */
/********************************************************************************/
int insert_uniquely_DEHT ( DEHT *ht, const unsigned char *key, int keyLength, 
				 const unsigned char *data, int dataLength);

/********************************************************************************/
/* Function add_DEHT inserts an ellement,                                       */
/*    whenever exists or not                                                    */
/* Inputs: DEHT to insert into, key and data (as binary buffer with size)       */
/* Output: just status of action:                                               */
/* If successfully insert returns DEHT_STATUS_SUCCESS.                          */
/* If fail, returns DEHT_STATUS_FAIL                                            */
/* Notes:                                                                       */
/* if hashPointersForLastBlockImageInMemory!=NULL use it (save "fseek" commands)*/
/* if anLastBlockSize not null use it either.                                   */
/* if hashTableOfPointersImageInMemory use it (less efficient but stil helps)   */
/* if both null, do not load table of pointers into memory just make simple     */
/* insert using several fseek when necessary.                                   */
/********************************************************************************/
int add_DEHT ( DEHT *ht, const unsigned char *key, int keyLength, 
				 const unsigned char *data, int dataLength);

/********************************************************************************/
/* Function query_DEHT query a key.                                             */
/* Inputs: DEHT to query in, key input and data output buffer.                  */
/* Output:                                                                      */
/* If successfully insert returns number of bytes fullfiled in data buffer      */
/* If not found returns DEHT_STATUS_NOT_NEEDED                                  */
/* If fail returns DEHT_STATUS_FAIL                                             */
/* Notes:                                                                       */
/* If hashTableOfPointersImageInMemory!=NULL use it to save single seek.        */
/* Else access using table of pointers on disk.                                 */
/* "ht" argument is non const as fseek is non const too (will change "keyFP")   */
/********************************************************************************/
int query_DEHT ( DEHT *ht, const unsigned char *key, int keyLength, 
				 const unsigned char *data, int dataMaxAllowedLength);

/************************************************************************************/
/* Function read_DEHT_pointers_table loads pointer of tables from disk into RAM     */
/* It will be used for effciency, e.g. when many queries expected soon              */
/* Input: DEHT to act on. (will change member hashTableOfPointersImageInMemory).    */
/* Output:                                                                          */
/* If it is already cached, do nothing and return DEHT_STATUS_NOT_NEEDED.           */
/* If fail, return DEHT_STATUS_FAIL, if success return DEHT_STATUS_NOT_SUCCESS      */
/************************************************************************************/
int read_DEHT_pointers_table(DEHT *ht); 

/************************************************************************************/
/* Function write_DEHT_pointers_table writes pointer of tables RAM to Disk & release*/
/* Input: DEHT to act on.                                                           */
/* Output:                                                                          */
/* If not RAM pointer is NULL, return DEHT_STATUS_NOT_NEEDED                        */
/* if fail return DEHT_STATUS_FAIL, if success return DEHT_STATUS_SUCCESS           */
/* Note: do not forget to use "free" and put NULL.                                  */
/************************************************************************************/
int write_DEHT_pointers_table(DEHT *ht); 

/************************************************************************************/
/* Function calc_DEHT_last_block_per_bucket calculate all rear pointers on key file */
/*   to enable insertion with a single seek. Will be called by user when many insert*/
/*   calls are expected. Note that these has no parallel on disk thus no "write"    */
/* Input: DEHT to act on (modify hashPointersForLastBlockImageInMemory)             */
/* Output:                                                                          */
/* If it is already exist, do nothing and return DEHT_STATUS_NOT_NEEDED.            */
/* If fail, return DEHT_STATUS_FAIL, if success return DEHT_STATUS_NOT_SUCCESS      */
/************************************************************************************/
int calc_DEHT_last_block_per_bucket(DEHT *ht); 

DEHT_DISK_PTR DEHT_findFirstBlockForBucket(DEHT * ht, ulong_t bucketIndex);
DEHT_DISK_PTR DEHT_findLastBlockForBucketDumb(DEHT * ht, ulong_t bucketIndex);
DEHT_DISK_PTR DEHT_allocKeyBlock(DEHT * ht);

bool_t DEHT_allocEmptyLocationInBucket(DEHT * ht, ulong_t bucketIndex,
					     byte_t * blockDataOut, ulong_t blockDataLen,
					     DEHT_DISK_PTR * blockDiskPtr, ulong_t * firstFreeIndex);

bool_t DEHT_writeData(DEHT * ht, const byte_t * data, ulong_t dataLen, 
		      DEHT_DISK_PTR * newDataOffset);

bool_t DEHT_readDataAtOffset(DEHT * ht, DEHT_DISK_PTR dataBlockOffset, 
			     byte_t * data, ulong_t dataMaxAllowedLength, ulong_t * bytesRead);


int DEHT_queryInternal(DEHT *ht, const unsigned char *key, int keyLength, const unsigned char *data, int dataMaxAllowedLength,
			byte_t * keyBlockOut, ulong_t keyBlockSize, DEHT_DISK_PTR * keyBlockDiskOffset, ulong_t * keyIndex, DEHT_DISK_PTR * lastKeyBlockDiskOffset);

/************************************************************************************/
/* Function lock_DEHT_files closes the DEHT files and release memory.               */
/* Input: DEHT to act on. No Output (never fail).                                   */
/* Notes:                                                                           */
/* calls write_DEHT_hash_table if necessary, call "free" when possible.             */
/* use "fclose" command. do not free "FILE *"                                       */
/************************************************************************************/
void lock_DEHT_files(DEHT *ht);
       
#endif
/************************* EOF (DEHT.h) ****************/ 
