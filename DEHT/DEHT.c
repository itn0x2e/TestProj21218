
#include "DEHT.h"
#include "../common/types.h"
#include "../common/constants.h"
#include "../common/utils.h"
#include "../common/io.h"




/************************************************  Internal function declarations *********************************************/


/**
* Function brief description: Initialize the DEHT instance (consolidation of shared init operation
*			      to minimize code duplication). Used by both c'tors.
* Function desc: This function will allocate a DEHT instance and take care of all 
*		 operations which are shared by create_empty_DEHT() and load_DEHT_from_files().
*		 It will create the formatted strings for both file names and open the corresponding
*		 files with the wanted file mode, as well as initializing both hash functions.
*
* @param prefix - filename prefix for DEHT files
* @param fileMode - mode to use for fopen(). Note that for modes begining with 'c' (for 'create'),
*		    an attempt is made to see if the files are present before doing anything.
*		    if the files are present, an error message is displayed and the operation 
*		    is aborted. Otherwise, the mode string is copied and the first char is changed
*		    to 'w' (for 'write').
* @param hashfun - function pointer for key-to-bucket-id function.
* @param validfun - function pointer for validation key generation function
*
* @ret NULL on error, a pointer to a new DEHT instance if successful.
*
*/
static DEHT * DEHT_initInstance (const char * prefix, char * fileMode, 
			   hashKeyIntoTableFunctionPtr hashfun, hashKeyforEfficientComparisonFunctionPtr validfun);

/**
* Function brief description: Free all system resources used by the supplied DEHT instance
* Function desc: This function makes no DEHT calls to flush pending data to disk,
*		 but it will call fflush() on both file objects before closing them.
*		 Afterwards, any pointers to allocated buffers in the DEHT instance 
*		 that are not NULL will be freed.
*
* @param ht - hash table object
*
* @ret None
*
*/
static void DEHT_freeResources(DEHT * instance, bool_t removeFiles);


/**
* Function brief description: Internal worker function for query operation.
* Function desc: This function implements the actual work needed to perform a query operation.
*		 In addition to the basic query operation, it returns extra information
*		 that is useful in the event of an update.
*
* @param ht - hash table object
* @param key - pointer to key to hash (binary, null termination isn't taken into account here)
* @param keyLength - length of key, in bytes
* @param data - buffer to store the corresponding data in (binary, not neccessarily null-terminated)
* @param ht - dataMaxAllowedLength - size of data buffer, in bytes
* @param keyBlockOut - this buffer will be used to read a block during this function.
*		       upon return, this buffer will hold the contents of the last block
*		       in the search. If a following update operation is to be performed,
*		       this block can be written to the keyBlockOffset in the key file,
*		       after updating it as needed
* @param keyBlockDiskOffset - will receive the disk offset in the key file for the last block
* @param keyIndex - If a match was found, this value represents the index of the sub record
*		    in the block that matched for key.
*
* @ret DEHT_STATUS_SUCCESS if a match was found, DEHT_STATUS_NOT_NEEDED if the match was not found,
*      DEHT_STATUS_FAIL if an error has occurred.
*
*/
static int DEHT_queryEx(DEHT *ht, const unsigned char *key, int keyLength, const unsigned char *data, int dataMaxAllowedLength,
			byte_t * keyBlockOut, ulong_t keyBlockSize, DEHT_DISK_PTR * keyBlockDiskOffset, ulong_t * keyIndex);




/**
* Function brief description: find disk offset to the first block for the wanted bucket
* Function desc: Meant as an abstraction layer for support of first block ptr caching.
*		 If the cache is loaded, it will be used. Otherwise, the value is 
*		 read from disk.
*
* @param ht - hash table object
* @param bucketIndex - index of bucket to get offset for 
* @param blockOffset - out parameter, will receive offset to the first block in key file
*
* @ret TRUE on success, FALSE otherwise
*/
static bool_t DEHT_findFirstBlockForBucket(DEHT * ht, ulong_t bucketIndex, DEHT_DISK_PTR * blockOffset);


/**
* Function brief description: Find the offset to the first block for the specified bucket.
*			      If no such block exists (first allocation for this bucket),
*			      allocate a new block
* Function desc: This function uses DEHT_findFirstBlockForBucket() to find the first block
*		 for the specified bucket. If the current offset is 0, a new block is allocated
*		 using DEHT_allocKeyBlock().
*
* @param ht - hash table object
* @param bucketIndex - index of bucket to get offset for 
* @param blockOffset - out parameter, will receive offset to the first block in key file
*
* @ret TRUE on success, FALSE otherwise
*/
static bool_t DEHT_findFirstBlockForBucketAndAlloc(DEHT * ht, ulong_t bucketIndex, DEHT_DISK_PTR * blockOffset);


/**
* Function brief description: Find the last block for the specified bucket directly, i.e. by
*			      'walking' the block list for this bucket and finding the last
*			      block in the list
* Function desc: This function uses DEHT_findFirstBlockForBucket() to retrieve the first block,
*		 then scans the linked list of key blocks for the first block pointing to 0 (invalid block ptr)
*
* @param ht - hash table object
* @param bucketIndex - index of bucket to get offset for 
* @param lastblockOffset - out parameter, will receive offset to the first block in key file
*
* @ret TRUE on success, FALSE otherwise
*/
static bool_t DEHT_findLastBlockForBucketDumb(DEHT * ht, ulong_t bucketIndex, DEHT_DISK_PTR * lastBlockOffset);

/**
* Function brief description: Find the last block for the specified bucket efficiently.
* Function desc: This function uses the last block ptr cache if present, or DEHT_findLastBlockDumb() 
*		 otherwise.
*
* @param ht - hash table object
* @param bucketIndex - index of bucket to get offset for 
* @param lastblockOffset - out parameter, will receive offset to the first block in key file
*
* @ret TRUE on success, FALSE otherwise
*/
static bool_t DEHT_findLastBlockForBucket(DEHT * ht, ulong_t bucketIndex, DEHT_DISK_PTR * lastBlockOffset);

/**
* Function brief description: Find an empty record at the end of the bucket's linked list
* Function desc: This function uses DEHT_findLastBlockForBucket() to find the last block,
*		 then scans it for an empty location. If the last block is full, a new
*		 block is allocated and inserted into the linked list.
*
* @param ht - hash table object
* @param bucketIndex - index of bucket to get offset for 
* @param blockDataOut - out parameter, used as buffer for reading the block while scanning. 
*			upon return, will hold the contents of the block where insertion is possible
* @param blockDataLen - size of the buffer pointed to by blockDataOut, in bytes
* @param blockDiskPtr - out parameter, will hold the disk offset of the block where insertion is possible
* @param firstFreeIndex - out parameter, will hold the first index where insertion is possible.
*
* @ret TRUE on success, FALSE otherwise
*
*/
static bool_t DEHT_allocEmptyLocationInBucket(DEHT * ht, ulong_t bucketIndex,
					     byte_t * blockDataOut, ulong_t blockDataLen,
					     DEHT_DISK_PTR * blockDiskPtr, ulong_t * firstFreeIndex);

/**
* Function brief description: allocate a new key block in the key file.
* Function desc: This function will "allocate" a new key block by appending KEY_FILE_BLOCK_SIZE(ht)
*		 zeroed bytes to the end of key file, and returning the offset of the begining of 
*		 the new block.
*
* @param ht - hash table object
*
* @ret Offset to the new block if successful, 0 otherwise (0 IS NOT a valid key block offset)
*
*/
static DEHT_DISK_PTR DEHT_allocKeyBlock(DEHT * ht);



/**
* Function brief description: Write a data chunk to the DEHT datastore.
* Function desc: This function will write the data in the 'data' buffer to disk
*		 the end of the data file, encapsulating it in the following manner:
		 the data length is stored as a one byte length indicator,
*		 then followed by 'length' bytes of data (the data store does not 
*		 care about null termination).
*		---------------------------------------------------------------
*		| LENGTH (1 byte) |   <<<<--- DATA BYTES (up to 255) --->>>>  |
*		---------------------------------------------------------------
* 		 
* @param ht - hash table object
* @param data - pointer to output buffer 
* @param dataLen - number of bytes to write
* @param newDataOffset - will receive the disk offset in the data file for the created chunk
* @ret DEHT_STATUS_SUCCESS on successful dump to disk, DEHT_STATUS_NOT_NEEDED if the buffer isn't
*      allocated or if numUnreleatedBytesSaved is 0. DEHT_STATUS_FAIL is returned on failure.
*
* @ret TRUE on success, FALSE otherwise
*/
static bool_t DEHT_addData(DEHT * ht, const byte_t * data, ulong_t dataLen, 
		      DEHT_DISK_PTR * newDataOffset);

/**
* Function brief description: read a data chunk from the DEHT datastore
* Function desc: Inside the data file, byte arrays are stored as a one byte length indicator,
*		 followed by 'length' bytes of data (the data store does not care about null
*		 termination). Illustration:
*		---------------------------------------------------------------
*		| LENGTH (1 byte) |   <<<<--- DATA BYTES (up to 255) --->>>>  |
*		---------------------------------------------------------------
*
* @note This function should been static (~private). It is only exposed for the purpose
*	of the textual dumping of the rainbow table
*
* @param ht - hash table object
* @param dataBlockOffset - offset in the data file to read from
* @param data - pointer to output buffer 
* @param dataMaxAllowedLength - size of data buffer, in bytes
* @param bytesRead - will receive the number of bytes copied to the buffer. Note that if 
*		     bytesRead == dataMaxAllowedLength, there may be more bytes pending. 
*		     You may want to make the call again with a bigger buffer in that case.
*
* @ret DEHT_STATUS_SUCCESS on successful dump to disk, DEHT_STATUS_NOT_NEEDED if the buffer isn't
*      allocated or if numUnreleatedBytesSaved is 0. DEHT_STATUS_FAIL is returned on failure.
*
*/
static bool_t DEHT_readDataAtOffset(DEHT * ht, DEHT_DISK_PTR dataBlockOffset, 
			     byte_t * data, ulong_t dataMaxAllowedLength, ulong_t * bytesRead);




/**
* Function brief description: Utility function to fill in the strings for the two filenames
*			      in the DEHT instance header.
* Function desc: This function fills in the sKeyfileName and sDatafileName field in the 
*		 DEHT instance, according to the given prefix.
*
* @param ht - hash table object
* @param prefix - file name prefix (.data or .key is added accordingly)
*
* @ret None (can only fail due to mis-use)
*
*/
static void DEHT_formatFilenames(DEHT * ht, char * prefix);

/**
* Function brief description: clean DEHT's files (according to the DEHT instance's 
* Function desc: Using the paths in the sKeyfileName and sDatafileName fields of the instance's 
* 		 struct, this function tests whether each file exists and deletes it using removeFile() 
*		 (see common/io.h)
*
* @param ht - hash table object
*
* @ret TRUE on success, FALSE otherwise
*
*/
static bool_t DEHT_removeFilesInternal(DEHT * ht);



/**
* Function brief description: call the callback for each record inside the block
* Function desc: This function calls the user defined callback once for each valid record
*		 in the key block given to it.
*		 The callback is supplied with the corresponding bucket index, key and data.
*
* @param ht - hash table object
* @param blockBuffer - a buffer containing the contents of the block to be iterated upon
* @param bucketIndex - bucket index in the hash table (needed to pass to the callback)
* @param callback - a call back to call (must not be null)
* @param param - generic parameter for use by the callback (can be null)
*
* @ret TRUE on success, FALSE otherwise
*
*/
static bool_t DEHT_enumerateBlock(DEHT * ht, 
				byte_t * blockBuffer,
				int bucketIndex,
				DEHT_enumerationCallback_t callback, void * param);


/**
* Function brief description: call the callback for each record inside the chosen bucket
* Function desc: This function uses DEHT_enumerateBlock() to call the user defined callback 
*		 once for each valid record in each key block in the bucket given to it.
*		 The callback is supplied with the corresponding bucket index, key and data.
*
* @param ht - hash table object
* @param blockBuffer - scratch-pad buffer for reading the contents of key file blocks.
*		       MUST BE THE SIZE OF THE DEHT block (see KEY_FILE_BLOCK_SIZE(ht)).
*		       this parameter is supplied here to spare the need to alloc and free
*		       this buffer for every call internally.
* @param bucketIndex - bucket index in the hash table (needed to pass to the callback)
* @param callback - a call back to call (must not be null)
* @param param - generic parameter for use by the callback (can be null)
*
* @ret TRUE on success, FALSE otherwise
*
*/
static bool_t DEHT_enumerateBucket(DEHT * ht, int bucketIndex, 
			    byte_t * blockBuffer,
			    DEHT_enumerationCallback_t callback, void * param);





/************************************************  Function defintions *********************************************/


DEHT * create_empty_DEHT(const char *prefix,
                        hashKeyIntoTableFunctionPtr hashfun, hashKeyforEfficientComparisonFunctionPtr validfun,
                        const char *dictName,
                        int numEntriesInHashTable, int nPairsPerBlock, int nBytesPerKey,
			int nUserBytes)
{
	bool_t errorState = FALSE;

	DEHT * ht = NULL;

	TRACE_FUNC_ENTRY();

	/* sanity */
	CHECK(0 != numEntriesInHashTable);
	CHECK(0 != nPairsPerBlock);
	CHECK(0 != nBytesPerKey);

	ht = DEHT_initInstance(prefix, "c+b", hashfun, validfun);
	CHECK(NULL != ht);

	/* Do extra inits */
	ht->header.magic = DEHT_HEADER_MAGIC;
	SAFE_STRNCPY(ht->header.sDictionaryName, dictName, sizeof(ht->header.sDictionaryName));
	ht->header.numEntriesInHashTable = numEntriesInHashTable;
	ht->header.nPairsPerBlock = nPairsPerBlock;
	ht->header.nBytesPerValidationKey = nBytesPerKey;
	ht->header.numUnrelatedBytesSaved = nUserBytes;

	/* write header to disk */
	CHECK_MSG(ht->sKeyfileName, (1 == fwrite(&(ht->header), sizeof(ht->header), 1, ht->keyFP)));

	/* write (empty) pointer table to disk */
	CHECK_MSG(ht->sKeyfileName, (growFile(ht->keyFP, sizeof(DEHT_DISK_PTR) * numEntriesInHashTable)));

	/* write (empty) user data to disk */
	CHECK_MSG(ht->sKeyfileName, (growFile(ht->dataFP, ht->header.numUnrelatedBytesSaved)));



	goto LBL_CLEANUP;	

LBL_ERROR:
	errorState = TRUE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	if (errorState) {
		if (NULL != ht) {
			DEHT_freeResources(ht, TRUE);
			ht = NULL;
		}
	}

	TRACE_FUNC_EXIT();

	return ht;
}


DEHT * load_DEHT_from_files(const char *prefix, hashKeyIntoTableFunctionPtr hashfun, hashKeyforEfficientComparisonFunctionPtr validfun)
{
	bool_t errorState = FALSE;

	DEHT * ht = NULL;

	TRACE_FUNC_ENTRY();

	ht = DEHT_initInstance(prefix, "r+b", hashfun, validfun);
	CHECK(NULL != ht);

	/* load dict settings from file */
	CHECK_MSG(ht->sKeyfileName, (1 == fread(&(ht->header), sizeof(ht->header), 1, ht->keyFP)));
	CHECK_MSG("corrupted key file", (DEHT_HEADER_MAGIC == ht->header.magic));


	goto LBL_CLEANUP;	

LBL_ERROR:
	errorState = TRUE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	if (errorState) {
		if (NULL != ht) {
			DEHT_freeResources(ht, FALSE);
			ht = NULL;
		}
	}

	TRACE_FUNC_EXIT();

	return ht;
}




static DEHT * DEHT_initInstance (const char * prefix, char * fileMode, 
			   hashKeyIntoTableFunctionPtr hashfun, hashKeyforEfficientComparisonFunctionPtr validfun)
{
	bool_t filesAlreadyExist = FALSE;
	bool_t errorState = TRUE;
	bool_t deleteFilesOnError = FALSE;

	DEHT * ht = NULL;

	char tempFileMode[MAX_FILE_MODE_LEN] = {0};

	TRACE_FUNC_ENTRY();

	/* sanity */
	CHECK(NULL != prefix);
	CHECK(NULL != fileMode);
	CHECK(NULL != hashfun);
	CHECK(NULL != validfun);

	ht = malloc(sizeof(DEHT));
	CHECK_MSG("malloc", (NULL != ht));

	memset(ht, 0, sizeof(DEHT));

	SAFE_STRNCPY(ht->sKeyfileName, prefix, sizeof(ht->sKeyfileName));
	SAFE_STRNCAT(ht->sKeyfileName, KEY_FILE_EXT, sizeof(ht->sKeyfileName));

	SAFE_STRNCPY(ht->sDatafileName, prefix, sizeof(ht->sKeyfileName));
	SAFE_STRNCAT(ht->sDatafileName, DATA_FILE_EXT, sizeof(ht->sKeyfileName));

	/* Open key file. If file mode begins with 'c', first check that the file does not exist */
	SAFE_STRNCPY(tempFileMode, fileMode, sizeof(tempFileMode));
	if ('c' == tempFileMode[0]) {
		/* we were asked to make sure the files weren't already present first */
		ht->keyFP = fopen(ht->sKeyfileName, "rb");
		if (NULL != ht->keyFP) {
			deleteFilesOnError = FALSE;
			filesAlreadyExist = TRUE;
			fprintf(stderr, "Error: File \"%s\" already exist\n", ht->sKeyfileName);
		}

		ht->dataFP = fopen(ht->sDatafileName, "rb");
		if (NULL != ht->dataFP) {
			deleteFilesOnError = FALSE;
			filesAlreadyExist = TRUE;
			fprintf(stderr, "Error: File \"%s\" already exist\n", ht->sDatafileName);
		}

		/* fail if files already exist */
		CHECK(!filesAlreadyExist);

		/* that check passed. Now modify the file mode back to a standard one */
		tempFileMode[0] = 'w';
		/* From now on, if we fail, we'd like to clean up the files */
		deleteFilesOnError = TRUE;
	}

	/* Open key file */
	ht->keyFP = fopen(ht->sKeyfileName, tempFileMode);
	CHECK_MSG(ht->sKeyfileName, (NULL != ht->keyFP));

	/* Open data file */
	ht->dataFP = fopen(ht->sDatafileName, tempFileMode);
	CHECK_MSG(ht->sDatafileName, (NULL != ht->dataFP));
	
	ht->hashTableOfPointersImageInMemory = NULL;
	ht->hashPointersForLastBlockImageInMemory = NULL;

	ht->hashFunc = hashfun;
	ht->comparisonHashFunc = validfun;

	errorState = FALSE;
	goto LBL_CLEANUP;


LBL_ERROR:
	errorState = TRUE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	if (errorState) {
		if (NULL != ht) {
			DEHT_freeResources(ht, deleteFilesOnError);
		}
		ht = NULL;
	}

	TRACE_FUNC_EXIT();
	return ht;
}




int insert_uniquely_DEHT ( DEHT *ht, const unsigned char *key, int keyLength, 
				 const unsigned char *data, int dataLength)
{
	int ret = DEHT_STATUS_FAIL;

	byte_t * tempKeyBlock = NULL;
	DEHT_DISK_PTR keyBlockDiskOffset = 0;
	ulong_t keyIndex = 0;
	
	KeyFilePair_t * targetRecord = NULL;
	DEHT_DISK_PTR newDataOffset = 0;
	
	byte_t tempData[DEHT_MAX_DATA_LEN] = {0};

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != key);
	CHECK(NULL != data);

	/* allocate a buffer for DEHT_queryEx */
	tempKeyBlock = malloc(KEY_FILE_BLOCK_SIZE(ht));
	CHECK_MSG("malloc", (NULL != tempKeyBlock));
	
	ret = DEHT_queryEx(ht, key, keyLength, tempData, sizeof(tempData), tempKeyBlock, KEY_FILE_BLOCK_SIZE(ht), 
				  &keyBlockDiskOffset, &keyIndex);

	switch(ret) {
	case DEHT_STATUS_NOT_NEEDED:
		TRACE("simple insert");
		/* not found - just insert */
		CHECK(DEHT_STATUS_SUCCESS == add_DEHT (ht, key, keyLength, data, dataLength));

		ret = DEHT_STATUS_SUCCESS;

		break;

	case DEHT_STATUS_FAIL:
	default:
		if (0 >= ret) {
			/* internal error */
			goto LBL_ERROR;
		}


		/* if we got here, the key was found */
		TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): key already in DEHT. Updating record at %#x\n", __FILE__, __LINE__, __FUNCTION__, (uint_t) keyBlockDiskOffset));

		/* update data in the data file */
		CHECK(DEHT_addData(ht, data, dataLength, &newDataOffset)); 

		/* get the key record */
		targetRecord = GET_N_REC_PTR_IN_BLOCK(ht, tempKeyBlock, keyIndex);
		/* update key record */
		targetRecord->dataOffset = newDataOffset;

		/* update block on disk */
		CHECK_MSG(ht->sKeyfileName, (pfwrite(ht->keyFP, keyBlockDiskOffset, tempKeyBlock, KEY_FILE_BLOCK_SIZE(ht))));

		ret = DEHT_STATUS_NOT_NEEDED;
		
		break;
	}

	goto LBL_CLEANUP;	

LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	FREE(tempKeyBlock);

	TRACE_FUNC_EXIT();
	return ret;
}


int add_DEHT ( DEHT *ht, const unsigned char *key, int keyLength, 
				 const unsigned char *data, int dataLength)
{
	int ret = DEHT_STATUS_FAIL;

	int hashTableIndex = 0;

	byte_t * blockContent = NULL;
	DEHT_DISK_PTR keyBlockOffset = 0;
	ulong_t freeIndex = 0;

	KeyFilePair_t * targetRec = NULL;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != key);
	CHECK(NULL != data);

	/* calc hash for key */
	CHECK(NULL != ht->hashFunc);
	hashTableIndex = ht->hashFunc(key, keyLength, ht->header.numEntriesInHashTable);
	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): bucket index=%#x\n", __FILE__, __LINE__, __FUNCTION__, hashTableIndex));

	blockContent = malloc(KEY_FILE_BLOCK_SIZE(ht));
	CHECK_MSG("malloc", (NULL != blockContent));
	CHECK(DEHT_allocEmptyLocationInBucket(ht, hashTableIndex, blockContent, KEY_FILE_BLOCK_SIZE(ht),
					     &keyBlockOffset, &freeIndex));

	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): using block at %#x, index=%lu\n", __FILE__, __LINE__, __FUNCTION__, (uint_t) keyBlockOffset, freeIndex));

	targetRec = GET_N_REC_PTR_IN_BLOCK(ht, blockContent, freeIndex);

	/* calc validation key and fill in record */
	/*! Note: return value isn't checked since the spec failed to include details regarding the key 
		  validation function interface */
	CHECK(NULL != ht->comparisonHashFunc);
	(void) ht->comparisonHashFunc(key, keyLength, targetRec->key);

	/* write payload to data file */
	CHECK(DEHT_addData(ht, data, dataLength, &(targetRec->dataOffset)));

	/* write updated block to disk */
	CHECK_MSG(ht->sKeyfileName, (pfwrite(ht->keyFP, keyBlockOffset, blockContent, KEY_FILE_BLOCK_SIZE(ht))));

	TRACE("block updated");

	ret = DEHT_STATUS_SUCCESS;
	goto LBL_CLEANUP;	

LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	FREE(blockContent);

	TRACE_FUNC_EXIT();
	return ret;
}


int query_DEHT ( DEHT *ht, const unsigned char *key, int keyLength, 
				 const unsigned char *data, int dataMaxAllowedLength)
{
	int ret = DEHT_STATUS_FAIL;

	byte_t * tempKeyBlock = NULL;
	DEHT_DISK_PTR keyBlockDiskOffset = 0;
	ulong_t keyIndex = 0;
	

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != key);
	CHECK(NULL != data);

	/* allocate a buffer for DEHT_queryEx */
	tempKeyBlock = malloc(KEY_FILE_BLOCK_SIZE(ht));
	CHECK_MSG("malloc", (NULL != tempKeyBlock));
	
	ret = DEHT_queryEx(ht, key, keyLength, data, dataMaxAllowedLength, tempKeyBlock, KEY_FILE_BLOCK_SIZE(ht), 
				  &keyBlockDiskOffset, &keyIndex);

	goto LBL_CLEANUP;	

LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	FREE(tempKeyBlock);

	TRACE_FUNC_EXIT();
	return ret;
}




static int DEHT_queryEx(DEHT *ht, const unsigned char *key, int keyLength, const unsigned char *data, int dataMaxAllowedLength,
			byte_t * keyBlockOut, ulong_t keyBlockSize, DEHT_DISK_PTR * keyBlockDiskOffset, ulong_t * keyIndex)
{
	int ret = DEHT_STATUS_FAIL;

	int hashTableIndex = 0;
	byte_t * validationKey = NULL;

	KeyFilePair_t * currPair = NULL;

	DEHT_DISK_PTR dataBlockOffset = 0;
	ulong_t bytesRead = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != key);
	CHECK(NULL != data);

	CHECK(NULL != keyBlockOut);
	CHECK(KEY_FILE_BLOCK_SIZE(ht) <= keyBlockSize);

	CHECK(NULL != keyBlockDiskOffset);
	CHECK(NULL != keyIndex);

	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): key=%s\n", __FILE__, __LINE__, __FUNCTION__, key));

	/* calc hash for key */
	CHECK(NULL != ht->hashFunc);
	hashTableIndex = ht->hashFunc(key, keyLength, ht->header.numEntriesInHashTable);
	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): bucket index=%#x\n", __FILE__, __LINE__, __FUNCTION__, (uint_t) hashTableIndex));


	CHECK(DEHT_findFirstBlockForBucketAndAlloc(ht, hashTableIndex, keyBlockDiskOffset));
	CHECK(0 != *keyBlockDiskOffset);

	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): first block for bucket %d at offset=%#x\n", __FILE__, __LINE__, __FUNCTION__, hashTableIndex, (uint_t) *keyBlockDiskOffset));

	/* If there is no block for this bucket, return with nothing */
	if (0 == *keyBlockDiskOffset) {
		ret = DEHT_STATUS_NOT_NEEDED;
		goto LBL_CLEANUP;
	}


	/* alloc space and calc validation key */
	validationKey = malloc(ht->header.nBytesPerValidationKey);
	CHECK_MSG("malloc", (NULL != validationKey));

	/*! Note: return value isn't checked since the spec failed to include details regarding the key 
		  validation function interface */
	CHECK(NULL != ht->comparisonHashFunc);
	(void) ht->comparisonHashFunc(key, keyLength, validationKey);

	while (0 != *keyBlockDiskOffset) {
		/* read block to mem */
		CHECK_MSG(ht->sKeyfileName, (pfread(ht->keyFP, *keyBlockDiskOffset, keyBlockOut, KEY_FILE_BLOCK_SIZE(ht))));

		/* scan this block */
		for(*keyIndex = 0;  *keyIndex < ht->header.nPairsPerBlock;  ++*keyIndex) {

			currPair = GET_N_REC_PTR_IN_BLOCK(ht, keyBlockOut, *keyIndex);

			if (0 == memcmp(currPair->key, validationKey, ht->header.nBytesPerValidationKey)) {
				break;
			}	
		}

		if (*keyIndex < ht->header.nPairsPerBlock) {
			break;
		}

		/* disk offset of the next pointer is the last element in the block */
		*keyBlockDiskOffset = *( (DEHT_DISK_PTR *) (keyBlockOut + KEY_FILE_BLOCK_SIZE(ht) - sizeof(DEHT_DISK_PTR)) );
		TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): next ptr from disk: %#x\n", __FILE__, __LINE__, __FUNCTION__, (uint_t) *keyBlockDiskOffset));
	}

	dataBlockOffset = 0;
	if (*keyIndex < ht->header.nPairsPerBlock) {
		dataBlockOffset = currPair->dataOffset;
	}
	else {
		/* we scanned everything, but found no matching key */
		ret = DEHT_STATUS_NOT_NEEDED;
		TRACE("key not found");
		goto LBL_CLEANUP;
	}

	bytesRead = 0;
	CHECK(DEHT_readDataAtOffset(ht, dataBlockOffset, (byte_t *) data, dataMaxAllowedLength, &bytesRead));

	TRACE("key found");

	ret = bytesRead;
	goto LBL_CLEANUP;
	
LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	FREE(validationKey);

	TRACE_FUNC_EXIT();
	return ret;
}





int DEHT_getUserBytes(DEHT * ht, byte_t * * bufPtr, ulong_t * bufSize)
{
	int ret = DEHT_STATUS_FAIL;

	TRACE_FUNC_ENTRY();
	CHECK(NULL != ht);
	CHECK(NULL != bufPtr);
	CHECK(NULL != bufSize);

	if (NULL != ht->userBuf) {
		*bufPtr = ht->userBuf;
		*bufSize = ht->header.numUnrelatedBytesSaved;
	
		ret = DEHT_STATUS_NOT_NEEDED;
		goto LBL_CLEANUP;
	}

	ht->userBuf = malloc(ht->header.numUnrelatedBytesSaved);
	CHECK_MSG("malloc", (NULL != ht->userBuf));

	CHECK_MSG(ht->sDatafileName, (pfread(ht->dataFP, DATA_FILE_OFFSET_TO_USER_BYTES, ht->userBuf, ht->header.numUnrelatedBytesSaved)));

	*bufSize = ht->header.numUnrelatedBytesSaved;
	*bufPtr = ht->userBuf;

	ret = DEHT_STATUS_SUCCESS;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	if (DEHT_STATUS_FAIL == ret) {
		FREE(ht->userBuf);
	}

	TRACE_FUNC_EXIT();
	return ret;
}




int DEHT_writeUserBytes(DEHT * ht)
{
	bool_t ret = DEHT_STATUS_FAIL;

	TRACE_FUNC_ENTRY();
	CHECK(NULL != ht);

	if ((NULL == ht->userBuf) || (0 == ht->header.numUnrelatedBytesSaved)) {
		ret = DEHT_STATUS_NOT_NEEDED;
		goto LBL_CLEANUP;
	}

	CHECK_MSG(ht->sDatafileName, (pfwrite(ht->dataFP, DATA_FILE_OFFSET_TO_USER_BYTES, ht->userBuf, ht->header.numUnrelatedBytesSaved)));

	/*! Note that unlink write_DEHT_pointers_table(), we DO NOT free the block here. Since this interface was
	    not dictated by the project spec, we chose what we found to be a more convenient interface for our use-case,
	    namely - only writing the data to disk, but not freeing the reference here !*/

	ret = DEHT_STATUS_SUCCESS;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}





int read_DEHT_pointers_table(DEHT *ht)
{
	int ret = DEHT_STATUS_FAIL;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);

	if (NULL != ht->hashTableOfPointersImageInMemory) {
		return DEHT_STATUS_NOT_NEEDED;
	}

	/* alloc cache */
	ht->hashTableOfPointersImageInMemory = malloc(KEY_FILE_FIRST_BLOCK_PTRS_SIZE(ht));
	CHECK_MSG("malloc", (NULL != ht->hashTableOfPointersImageInMemory));

	/* read the offset table */
	CHECK_MSG(ht->sKeyfileName, (pfread(ht->keyFP, KEY_FILE_OFFSET_TO_FIRST_BLOCK_PTRS(ht), (byte_t *) ht->hashTableOfPointersImageInMemory, KEY_FILE_FIRST_BLOCK_PTRS_SIZE(ht))));

	ret = DEHT_STATUS_SUCCESS;
	goto LBL_CLEANUP;

LBL_ERROR:
	/* on error, free the buffer */
	FREE(ht->hashTableOfPointersImageInMemory);

	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}

int write_DEHT_pointers_table(DEHT *ht)
{
	int ret = DEHT_STATUS_FAIL;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);

	if (NULL == ht->hashTableOfPointersImageInMemory) {
		return DEHT_STATUS_NOT_NEEDED;
	}

	/* write the offset table */
	CHECK_MSG(ht->sKeyfileName, (pfwrite(ht->keyFP, KEY_FILE_OFFSET_TO_FIRST_BLOCK_PTRS(ht), (byte_t *) ht->hashTableOfPointersImageInMemory, KEY_FILE_FIRST_BLOCK_PTRS_SIZE(ht))));

	/*! The project spec demanded that we free the cache here. It would be better to do so only on destruction, but nevermind */
	FREE(ht->hashTableOfPointersImageInMemory);

	ret = DEHT_STATUS_SUCCESS;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}

int calc_DEHT_last_block_per_bucket(DEHT *ht)
{
	int ret = DEHT_STATUS_FAIL;
	size_t rawTableSize = 0;
	ulong_t bucketIndex = 0;
	int pointerTableLoadRes = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);

	if (NULL != ht->hashPointersForLastBlockImageInMemory) {
		return DEHT_STATUS_NOT_NEEDED;
	}

	/* We will be scanning the entire bucket table. It would be wise to use the 
	  first block cache for this step */
	pointerTableLoadRes = read_DEHT_pointers_table(ht);

	/* alloc cache */
	rawTableSize = ht->header.numEntriesInHashTable * sizeof(DEHT_DISK_PTR);
	ht->hashPointersForLastBlockImageInMemory = malloc(rawTableSize);
	CHECK_MSG("malloc", (NULL != ht->hashPointersForLastBlockImageInMemory));

	for (bucketIndex = 0;  bucketIndex < ht->header.numEntriesInHashTable;  ++bucketIndex) {
		CHECK(DEHT_findLastBlockForBucketDumb(ht, bucketIndex, ht->hashPointersForLastBlockImageInMemory + bucketIndex));
	}

	ret = DEHT_STATUS_SUCCESS;
	goto LBL_CLEANUP;

LBL_ERROR:
	/* free on error */
	FREE(ht->hashPointersForLastBlockImageInMemory);

	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	/* If the first block pointer table wasn't loaded before us, we should unload it (errors are silenced) */
	if (DEHT_STATUS_SUCCESS == pointerTableLoadRes) {
		(void) write_DEHT_pointers_table(ht);
	}

	TRACE_FUNC_EXIT();
	return ret;
}


static bool_t DEHT_findFirstBlockForBucket(DEHT * ht, ulong_t bucketIndex, DEHT_DISK_PTR * blockOffset)
{
	bool_t ret = FALSE;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != blockOffset);

	CHECK(bucketIndex < ht->header.numEntriesInHashTable);

	if (NULL != ht->hashTableOfPointersImageInMemory) {
		*blockOffset = ht->hashTableOfPointersImageInMemory[bucketIndex];
	}
	else {
		CHECK_MSG(ht->sKeyfileName, (pfread(ht->keyFP, KEY_FILE_OFFSET_TO_FIRST_BLOCK_PTRS(ht) + bucketIndex * sizeof(DEHT_DISK_PTR), (byte_t *) blockOffset, sizeof(*blockOffset))));
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	*blockOffset = 0;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}

static bool_t DEHT_findFirstBlockForBucketAndAlloc(DEHT * ht, ulong_t bucketIndex, DEHT_DISK_PTR * blockOffset)
{
	bool_t ret = FALSE;
	
	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != blockOffset);
	CHECK(bucketIndex < ht->header.numEntriesInHashTable);


	CHECK(DEHT_findFirstBlockForBucket(ht, bucketIndex, blockOffset));

	/* if this is the very first block for this bucket, alloc a new one */
	if (0 == *blockOffset) {

		*blockOffset = DEHT_allocKeyBlock(ht);
		CHECK(0 != *blockOffset);

		/* if present, update first block cache */
		if (NULL != ht->hashTableOfPointersImageInMemory) {
			/* update cache ptr */
			ht->hashTableOfPointersImageInMemory[bucketIndex] = *blockOffset;
		}
		else {
			/* update on-disk ptr */
			CHECK_MSG(ht->sKeyfileName, (pfwrite(ht->keyFP, KEY_FILE_OFFSET_TO_FIRST_BLOCK_PTRS(ht) + bucketIndex * sizeof(DEHT_DISK_PTR), (byte_t *) blockOffset, sizeof(*blockOffset))));
		}

		/* if present, update last block cache */
		if (NULL != ht->hashPointersForLastBlockImageInMemory) {
			ht->hashPointersForLastBlockImageInMemory[bucketIndex] = *blockOffset;
		}
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	*blockOffset = 0;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}




static bool_t DEHT_findLastBlockForBucketDumb(DEHT * ht, ulong_t bucketIndex, DEHT_DISK_PTR * lastBlockOffset)
{
	bool_t ret = FALSE;
	DEHT_DISK_PTR blockOffset = 0;
	
	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != lastBlockOffset);
	CHECK(bucketIndex < ht->header.numEntriesInHashTable);

	CHECK(DEHT_findFirstBlockForBucket(ht, bucketIndex, &blockOffset));
	if (0 == blockOffset) {
		*lastBlockOffset = 0;

		ret = TRUE;
		goto LBL_CLEANUP;
	}

	*lastBlockOffset = blockOffset;
	/* scan chain */
	while (0 != blockOffset) {
		*lastBlockOffset = blockOffset;
		
		/* read the offset to the next block from disk */
		CHECK_MSG(ht->sKeyfileName, (pfread(ht->keyFP, blockOffset + KEY_FILE_BLOCK_SIZE(ht) - sizeof(DEHT_DISK_PTR), (byte_t *) &blockOffset, sizeof(blockOffset))));
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	*lastBlockOffset = 0;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}



static bool_t DEHT_findLastBlockForBucket(DEHT * ht, ulong_t bucketIndex, DEHT_DISK_PTR * lastBlockOffset)
{
	bool_t ret = FALSE;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(bucketIndex < ht->header.numEntriesInHashTable);

	if ((NULL == ht->hashPointersForLastBlockImageInMemory) ||
	    (0 == ht->hashPointersForLastBlockImageInMemory[bucketIndex]) ) {
		TRACE("scanning chain");

		CHECK(DEHT_findLastBlockForBucketDumb(ht, bucketIndex, lastBlockOffset));
	}
	else {
		TRACE("using cache");
		*lastBlockOffset = ht->hashPointersForLastBlockImageInMemory[bucketIndex];
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	*lastBlockOffset = 0;
	ret = FALSE;

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}

static bool_t DEHT_allocEmptyLocationInBucket(DEHT * ht, ulong_t bucketIndex,
					     byte_t * blockDataOut, ulong_t blockDataLen,
					     DEHT_DISK_PTR * blockDiskPtr, ulong_t * firstFreeIndex)
{
	bool_t ret = FALSE;

	DEHT_DISK_PTR newBlock = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(bucketIndex < ht->header.numEntriesInHashTable);
	CHECK(NULL != blockDataOut);
	CHECK(KEY_FILE_BLOCK_SIZE(ht) <= blockDataLen);
	CHECK(NULL != blockDiskPtr);
	CHECK(NULL != firstFreeIndex);

	CHECK(DEHT_findLastBlockForBucket(ht, bucketIndex, blockDiskPtr));
	if (0 == *blockDiskPtr) {
		/* if this is the first ever block, call DEHT_findFirstBlockForBucketAndAlloc() which will also alloc the first block */
		CHECK(DEHT_findFirstBlockForBucketAndAlloc(ht, bucketIndex, blockDiskPtr));
		CHECK(0 != *blockDiskPtr);
	}

	/* read it */
	CHECK_MSG(ht->sKeyfileName, (pfread(ht->keyFP, *blockDiskPtr, blockDataOut, KEY_FILE_BLOCK_SIZE(ht))));

	/* get the current used block count */
	*firstFreeIndex = GET_USED_RECORD_COUNT(blockDataOut);
	
	/* see if this block is full */
	if (*firstFreeIndex >= ht->header.nPairsPerBlock) {
	
		newBlock = DEHT_allocKeyBlock(ht);
		CHECK(0 != newBlock);

		/* update old block */
		SET_NEXT_BLOCK_PTR(ht, blockDataOut, newBlock);
	
		/* update last block reference if cache is present */
		if (NULL != ht->hashPointersForLastBlockImageInMemory) {
			ht->hashPointersForLastBlockImageInMemory[bucketIndex] = newBlock;
		}
		
		CHECK_MSG(ht->sKeyfileName, (pfwrite(ht->keyFP, *blockDiskPtr, blockDataOut, KEY_FILE_BLOCK_SIZE(ht))));
		
		/* now, just return the reference to the start of the new block */
		memset(blockDataOut, 0, KEY_FILE_BLOCK_SIZE(ht));
		*blockDiskPtr = newBlock;
		*firstFreeIndex = 0;
	}

	/* increase used pairs count by one. If & when the user will update the block on disk, this will be updated as well */
	SET_USED_RECORD_COUNT(blockDataOut, GET_USED_RECORD_COUNT(blockDataOut) + 1);

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	*blockDiskPtr = 0;
	*firstFreeIndex = 0;
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}


static DEHT_DISK_PTR DEHT_allocKeyBlock(DEHT * ht)
{
	DEHT_DISK_PTR newBlock  = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);

	/* this is the first block - use the end of the file */
	(void) fflush(ht->keyFP);
	CHECK(0 == fseek(ht->keyFP, 0, SEEK_END));
	newBlock = ftell(ht->keyFP);
	
	/* alloc an empty block (init to NULLs) */
	CHECK(growFile(ht->keyFP, KEY_FILE_BLOCK_SIZE(ht)));

	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): allocated a block at %#x\n", __FILE__, __LINE__, __FUNCTION__, (uint_t) newBlock));

	goto LBL_CLEANUP;

LBL_ERROR:
	newBlock = 0;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return newBlock;
}


static bool_t DEHT_readDataAtOffset(DEHT * ht, DEHT_DISK_PTR dataBlockOffset, 
			     byte_t * data, ulong_t dataMaxAllowedLength, ulong_t * bytesRead)
{
	bool_t ret = FALSE;

	byte_t dataLen = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != data);
	CHECK(NULL != bytesRead);
	*bytesRead = 0;

	CHECK_MSG(ht->sDatafileName, (pfread(ht->dataFP, dataBlockOffset, &dataLen, sizeof(dataLen))));

	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): data size is %d\n", __FILE__, __LINE__, __FUNCTION__, dataLen));

	*bytesRead = fread(data, 1, MIN(dataMaxAllowedLength, dataLen), ht->dataFP);
	CHECK(0 <= *bytesRead);

	ret = TRUE;
	goto LBL_CLEANUP;
	
LBL_ERROR:
	*bytesRead = 0;
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;

}



static bool_t DEHT_addData(DEHT * ht, const byte_t * data, ulong_t dataLen, 
		      DEHT_DISK_PTR * newDataOffset)
{
	bool_t ret = FALSE;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != data);
	CHECK(NULL != newDataOffset);
	*newDataOffset = 0;

	if (dataLen > DEHT_MAX_DATA_LEN) {
		FAIL("cannot insert data larger than " STR_TOKENIZE(DEHT_MAX_DATA_LEN));
	}

	(void) fflush(ht->dataFP);
	CHECK(0 == fseek(ht->dataFP, 0, SEEK_END));
	*newDataOffset = ftell(ht->dataFP);

	CHECK(1 == fwrite(&dataLen, sizeof(byte_t), 1, ht->dataFP));
	if (0 != dataLen) {
		CHECK(1 == fwrite(data, dataLen, 1, ht->dataFP));
	}

	ret = TRUE;
	goto LBL_CLEANUP;
	
LBL_ERROR:
	*newDataOffset = 0;
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;

}





void lock_DEHT_files(DEHT *ht)
{

	TRACE_FUNC_ENTRY();

	/* If present, serialize pointer table to disk. Errors are ignored -
	   we try to do as much as we can in spite of error in the name of robustness */
	(void) write_DEHT_pointers_table(ht);

	/* If present, serialize user data to disk. Errors are ignored -
	   we try to do as much as we can in spite of error in the name of robustness */
	(void) DEHT_writeUserBytes(ht);

	DEHT_freeResources(ht, FALSE);
}
       

static void DEHT_freeResources(DEHT * ht, bool_t removeFiles)
{
	TRACE_FUNC_ENTRY();

	CHECK (NULL != ht);

	(void) fflush(ht->keyFP);
	FCLOSE(ht->keyFP);
	(void) fflush(ht->dataFP);
	FCLOSE(ht->dataFP);

	/* free ht cache if present */
	FREE(ht->hashTableOfPointersImageInMemory);
	FREE(ht->hashPointersForLastBlockImageInMemory);
	FREE(ht->userBuf);

	if (removeFiles) {
		/* attempt to remove bad files. Errors are silenced */
		CHECK(DEHT_removeFilesInternal(ht));
	}

	/* finally, free the ht itself */
	FREE(ht);

	goto LBL_CLEANUP;

LBL_ERROR:
	/* do nothing special - just quit */
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	/* bye */
	TRACE_FUNC_EXIT();
	return;
}

static void DEHT_formatFilenames(DEHT * ht, char * prefix)
{
	TRACE_FUNC_ENTRY();
	CHECK(NULL != ht);
	CHECK(NULL != prefix);

	SAFE_STRNCPY(ht->sKeyfileName, prefix, sizeof(ht->sKeyfileName));
	SAFE_STRNCAT(ht->sKeyfileName, KEY_FILE_EXT, sizeof(ht->sKeyfileName));

	SAFE_STRNCPY(ht->sDatafileName, prefix, sizeof(ht->sKeyfileName));
	SAFE_STRNCAT(ht->sDatafileName, DATA_FILE_EXT, sizeof(ht->sKeyfileName));

	goto LBL_CLEANUP;
	
LBL_ERROR:
	TRACE_FUNC_ERROR();
	/* Fail silently. Our caller will be able to handle this. */

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return;
}

static bool_t DEHT_removeFilesInternal(DEHT * ht)
{
	bool_t ret = FALSE;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);

	CHECK(removeFile(ht->sKeyfileName));
	CHECK(removeFile(ht->sDatafileName));

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}

bool_t DEHT_removeFiles(char * filenamePrefix)
{
	bool_t ret = FALSE;
	DEHT tempContainer;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != filenamePrefix);

	memset(&tempContainer, 0, sizeof(tempContainer));

	DEHT_formatFilenames(&tempContainer, filenamePrefix);

	CHECK(DEHT_removeFilesInternal(&tempContainer));

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}



static bool_t DEHT_enumerateBlock(DEHT * ht, 
				byte_t * blockBuffer,
				int bucketIndex,
				DEHT_enumerationCallback_t callback, void * param)
{
	bool_t ret = FALSE;

	ulong_t recordIndex = 0;
	KeyFilePair_t * currPair = NULL;
	byte_t currData[DEHT_MAX_DATA_LEN + 1];
	ulong_t bytesRead = 0;

	TRACE_FUNC_ENTRY();
	
	CHECK(NULL != ht);
	CHECK(NULL != blockBuffer);
	CHECK(NULL != callback);


	for (recordIndex = 0; recordIndex < GET_USED_RECORD_COUNT(blockBuffer); ++recordIndex) {
		currPair = GET_N_REC_PTR_IN_BLOCK(ht, blockBuffer, recordIndex);

		CHECK(DEHT_readDataAtOffset(ht, currPair->dataOffset, currData, sizeof(currData), &bytesRead));
		
		/* terminate for sure */
		currData[MIN(bytesRead, sizeof(currData) - 1)] = 0x00;

		/* call user callback */
		callback(bucketIndex, 
		     currPair->key, ht->header.nBytesPerValidationKey,
		     currData, bytesRead,
		     param);		
		
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;	
}



static bool_t DEHT_enumerateBucket(DEHT * ht, int bucketIndex, 
			    byte_t * blockBuffer,
			    DEHT_enumerationCallback_t callback, void * param)
{
	bool_t ret = FALSE;
	DEHT_DISK_PTR blockOffset = 0;

	TRACE_FUNC_ENTRY();
	CHECK(NULL != ht);
	CHECK(NULL != blockBuffer);
	CHECK(NULL != callback);

	CHECK(DEHT_findFirstBlockForBucket(ht, bucketIndex, &blockOffset));

	while (0 != blockOffset) {
		CHECK_MSG(ht->sKeyfileName, (pfread(ht->keyFP, blockOffset, blockBuffer, KEY_FILE_BLOCK_SIZE(ht))));
		
		CHECK(DEHT_enumerateBlock(ht, blockBuffer, bucketIndex, callback, param));
		
		blockOffset = GET_NEXT_BLOCK_PTR(ht, blockBuffer);
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;	
}


bool_t DEHT_enumerate(DEHT * ht, 
		      DEHT_enumerationCallback_t callback, void * param)
{
	bool_t ret = FALSE;

	int bucketIndex = 0;
	byte_t * currBlock = NULL;

	TRACE_FUNC_ENTRY();
	CHECK(NULL != ht);
	CHECK(NULL != callback);

	/* A buffer to hold a single block */
	currBlock = malloc(KEY_FILE_BLOCK_SIZE(ht));
	CHECK_MSG("malloc", (NULL != currBlock));

	for (bucketIndex = 0; bucketIndex < ht->header.numEntriesInHashTable; ++bucketIndex) {
		CHECK(DEHT_enumerateBucket(ht, bucketIndex, currBlock, callback, param));
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:
	FREE(currBlock);

	TRACE_FUNC_EXIT();
	return ret;
}









