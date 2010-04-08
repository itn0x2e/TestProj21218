
#include "DEHT.h"
#include "../common/types.h"
#include "../common/utils.h"
#include "../common/io.h"




DEHT *create_empty_DEHT(const char *prefix,
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
	ht->header.magic = DEHT_HEADER_MAGIC;

	/* write header to disk */
	CHECK(1 == fwrite(&(ht->header), sizeof(ht->header), 1, ht->keyFP));

	/* write (empty) pointer table to disk */
	CHECK(growFile(ht->keyFP, sizeof(DEHT_DISK_PTR) * numEntriesInHashTable));

	/* write (empty) user data to disk */
	CHECK(growFile(ht->dataFP, ht->header.numUnrelatedBytesSaved));



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
	CHECK(1 == fread(&(ht->header), sizeof(ht->header), 1, ht->keyFP));
	CHECK(DEHT_HEADER_MAGIC == ht->header.magic);


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




DEHT * DEHT_initInstance (const char * prefix, char * fileMode, 
			   hashKeyIntoTableFunctionPtr hashfun, hashKeyforEfficientComparisonFunctionPtr validfun)
{
	bool_t errorState = TRUE;
	bool_t deleteFilesOnError = FALSE;

	DEHT * ht = NULL;

	char tempFileMode[10] = {0};

	TRACE_FUNC_ENTRY();

	/* sanity */
	CHECK(NULL != prefix);
	CHECK(NULL != fileMode);
	CHECK(NULL != hashfun);
	CHECK(NULL != validfun);

	ht = malloc(sizeof(DEHT));
	CHECK(NULL != ht);

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
			FAIL("key file already exists!");
		}

		ht->dataFP = fopen(ht->sDatafileName, "rb");
		if (NULL != ht->dataFP) {
			deleteFilesOnError = FALSE;
			FAIL("data file already exists!");
		}

		/* that check passed. Now modify the file mode back to a standard one */
		tempFileMode[0] = 'w';
		/* From now on, if we fail, we'd like to clean up the files */
		deleteFilesOnError = TRUE;
	}

	/* Open key file */
	ht->keyFP = fopen(ht->sKeyfileName, tempFileMode);
	CHECK(NULL != ht->keyFP);
	/*! CHECK(0 == setvbuf(ht->keyFP, NULL, _IOFBF, 256)); !*/

	/* Open data file */
	ht->dataFP = fopen(ht->sDatafileName, tempFileMode);
	CHECK(NULL != ht->dataFP);
	/*! CHECK(0 == setvbuf(ht->dataFP, NULL, _IOFBF, 256)); !*/
	
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
	DEHT_DISK_PTR lastKeyBlockDiskOffset = 0;
	
	KeyFilePair_t * targetRecord = NULL;
	DEHT_DISK_PTR newDataOffset = 0;
	
	byte_t tempData[DEHT_DATA_MAX_LEN] = {0};

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != key);
	CHECK(NULL != data);

	/* allocate a buffer for DEHT_queryInternal */
	tempKeyBlock = malloc(KEY_FILE_BLOCK_SIZE(ht));
	CHECK(NULL != tempKeyBlock);
	
	ret = DEHT_queryInternal(ht, key, keyLength, tempData, sizeof(tempData), tempKeyBlock, KEY_FILE_BLOCK_SIZE(ht), 
				  &keyBlockDiskOffset, &keyIndex, &lastKeyBlockDiskOffset);

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
		TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): updating record at %#x\n", __FILE__, __LINE__, __FUNCTION__, (uint_t) keyBlockDiskOffset));

		/* write the new data to the data file */
		CHECK(DEHT_writeData(ht, data, dataLength, &newDataOffset));

		/* update the target record */
		targetRecord = GET_N_REC_PTR_IN_BLOCK(ht, tempKeyBlock, keyIndex);
		targetRecord->dataOffset = newDataOffset;

		/* update block on disk */
		CHECK(pfwrite(ht->keyFP, keyBlockDiskOffset, tempKeyBlock, KEY_FILE_BLOCK_SIZE(ht)));

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
	CHECK(NULL != blockContent);
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
	CHECK(DEHT_writeData(ht, data, dataLength, &(targetRec->dataOffset)));

	/* write updated block to disk */
	CHECK(pfwrite(ht->keyFP, keyBlockOffset, blockContent, KEY_FILE_BLOCK_SIZE(ht)));

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
	DEHT_DISK_PTR lastKeyBlockDiskOffset = 0;
	

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != key);
	CHECK(NULL != data);

	/* allocate a buffer for DEHT_queryInternal */
	tempKeyBlock = malloc(KEY_FILE_BLOCK_SIZE(ht));
	CHECK(NULL != tempKeyBlock);
	
	ret = DEHT_queryInternal(ht, key, keyLength, data, dataMaxAllowedLength, tempKeyBlock, KEY_FILE_BLOCK_SIZE(ht), 
				  &keyBlockDiskOffset, &keyIndex, &lastKeyBlockDiskOffset);

	goto LBL_CLEANUP;	

LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	FREE(tempKeyBlock);

	TRACE_FUNC_EXIT();
	return ret;
}





int DEHT_queryInternal(DEHT *ht, const unsigned char *key, int keyLength, const unsigned char *data, int dataMaxAllowedLength,
			byte_t * keyBlockOut, ulong_t keyBlockSize, DEHT_DISK_PTR * keyBlockDiskOffset, ulong_t * keyIndex, DEHT_DISK_PTR * lastKeyBlockDiskOffset)
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
	CHECK(NULL != lastKeyBlockDiskOffset);

	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): key=%s\n", __FILE__, __LINE__, __FUNCTION__, key));

	/* calc hash for key */
	CHECK(NULL != ht->hashFunc);
	hashTableIndex = ht->hashFunc(key, keyLength, ht->header.numEntriesInHashTable);
	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): bucket index=%#x\n", __FILE__, __LINE__, __FUNCTION__, (uint_t) hashTableIndex));

	if (NULL != ht->hashTableOfPointersImageInMemory) {
		*keyBlockDiskOffset = ht->hashTableOfPointersImageInMemory[hashTableIndex];
		TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): first ptr (from cache): %#x\n", __FILE__, __LINE__, __FUNCTION__, (uint_t) *keyBlockDiskOffset));
	}
	else {
		/* no cache - read from disk */
		CHECK(pfread(ht->keyFP, KEY_FILE_OFFSET_TO_FIRST_BLOCK_PTRS(ht) + hashTableIndex * sizeof(DEHT_DISK_PTR), (byte_t *) keyBlockDiskOffset, sizeof(*keyBlockDiskOffset)));
		TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): first ptr (from disk): %#x\n", __FILE__, __LINE__, __FUNCTION__, (uint_t) *keyBlockDiskOffset));
	}
	*lastKeyBlockDiskOffset = *keyBlockDiskOffset;

	/* If there is no block for this bucket, return with nothing */
	if (0 == *keyBlockDiskOffset) {
		ret = DEHT_STATUS_NOT_NEEDED;
		goto LBL_CLEANUP;
	}


	/* alloc space and calc validation key */
	validationKey = malloc(ht->header.nBytesPerValidationKey);
	CHECK(NULL != validationKey);

	/*! Note: return value isn't checked since the spec failed to include details regarding the key 
		  validation function interface */
	CHECK(NULL != ht->comparisonHashFunc);
	(void) ht->comparisonHashFunc(key, keyLength, validationKey);

	while (0 != *keyBlockDiskOffset) {
		/* read block to mem */
		CHECK(pfread(ht->keyFP, *keyBlockDiskOffset, keyBlockOut, KEY_FILE_BLOCK_SIZE(ht)));

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

		/* save old ptr */
		*lastKeyBlockDiskOffset = *keyBlockDiskOffset;

		/* disk offset of the next pointer is the last element in the block */
		*keyBlockDiskOffset = *( (DEHT_DISK_PTR *) (keyBlockOut + KEY_FILE_BLOCK_SIZE(ht) - sizeof(DEHT_DISK_PTR)) );
		TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): next ptr from disk: %#x\n", __FILE__, __LINE__, __FUNCTION__, (uint_t) *keyBlockDiskOffset));

		/*! TODO: update last block cache if present? !*/
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





int DEHT_readUserBytes(DEHT * ht, void ** bufPtr, ulong_t * bufSize)
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
	CHECK(NULL != ht->userBuf);

	CHECK(pfread(ht->dataFP, DATA_FILE_OFFSET_TO_USER_BYTES, ht->userBuf, ht->header.numUnrelatedBytesSaved));

	*bufSize = ht->header.numUnrelatedBytesSaved;
	*bufPtr = ht->userBuf;

	ret = DEHT_STATUS_SUCCESS;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	if (!ret) {
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

	if (NULL == ht->userBuf) {
		ret = DEHT_STATUS_NOT_NEEDED;
		goto LBL_CLEANUP;
	}

	CHECK(pfwrite(ht->dataFP, DATA_FILE_OFFSET_TO_USER_BYTES, ht->userBuf, ht->header.numUnrelatedBytesSaved));

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
	CHECK(NULL != ht->hashTableOfPointersImageInMemory);

	/* read the offset table */
	CHECK(pfread(ht->keyFP, KEY_FILE_OFFSET_TO_FIRST_BLOCK_PTRS(ht), (byte_t *) ht->hashTableOfPointersImageInMemory, KEY_FILE_FIRST_BLOCK_PTRS_SIZE(ht)));

	ret = DEHT_STATUS_SUCCESS;
	goto LBL_CLEANUP;

LBL_ERROR:
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
	CHECK(pfwrite(ht->keyFP, KEY_FILE_OFFSET_TO_FIRST_BLOCK_PTRS(ht), (byte_t *) ht->hashTableOfPointersImageInMemory, KEY_FILE_FIRST_BLOCK_PTRS_SIZE(ht)));

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

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);

	if (NULL != ht->hashPointersForLastBlockImageInMemory) {
		return DEHT_STATUS_NOT_NEEDED;
	}

	/* alloc cache */
	rawTableSize = ht->header.numEntriesInHashTable * sizeof(DEHT_DISK_PTR);
	ht->hashPointersForLastBlockImageInMemory = malloc(rawTableSize);
	CHECK(NULL != ht->hashPointersForLastBlockImageInMemory);

	for (bucketIndex = 0;  bucketIndex < ht->header.numEntriesInHashTable;  ++bucketIndex) {
		ht->hashPointersForLastBlockImageInMemory[bucketIndex] = DEHT_findLastBlockForBucketDumb(ht, bucketIndex);
	}

	ret = DEHT_STATUS_SUCCESS;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}


DEHT_DISK_PTR DEHT_findFirstBlockForBucket(DEHT * ht, ulong_t bucketIndex)
{
	DEHT_DISK_PTR blockOffset = 0;
	
	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(bucketIndex < ht->header.numEntriesInHashTable);

	if (NULL != ht->hashTableOfPointersImageInMemory) {
		blockOffset = ht->hashTableOfPointersImageInMemory[bucketIndex];
	}
	else {
		CHECK(pfread(ht->keyFP, KEY_FILE_OFFSET_TO_FIRST_BLOCK_PTRS(ht) + bucketIndex * sizeof(DEHT_DISK_PTR), (byte_t *) &blockOffset, sizeof(blockOffset)));
	}

	/* if this is the very first block, alloc a new one */
	if (0 == blockOffset) {
		blockOffset = DEHT_allocKeyBlock(ht);
		CHECK(0 != blockOffset);

		/* if present, update first & last block caches */
		if (NULL != ht->hashTableOfPointersImageInMemory) {
			/* update cache ptr */
			ht->hashTableOfPointersImageInMemory[bucketIndex] = blockOffset;
		}
		else {
			/* update on-disk ptr */
			CHECK(pfwrite(ht->keyFP, KEY_FILE_OFFSET_TO_FIRST_BLOCK_PTRS(ht) + bucketIndex * sizeof(DEHT_DISK_PTR), (byte_t *) &blockOffset, sizeof(blockOffset)));
		}

		if (NULL != ht->hashPointersForLastBlockImageInMemory) {
			ht->hashPointersForLastBlockImageInMemory[bucketIndex] = blockOffset;
		}
	}

	goto LBL_CLEANUP;

LBL_ERROR:
	blockOffset = 0;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return blockOffset;
}


DEHT_DISK_PTR DEHT_findLastBlockForBucketDumb(DEHT * ht, ulong_t bucketIndex)
{
	DEHT_DISK_PTR blockOffset = 0;
	DEHT_DISK_PTR lastBlockOffset = 0;
	
	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(bucketIndex < ht->header.numEntriesInHashTable);

	blockOffset = DEHT_findFirstBlockForBucket(ht, bucketIndex);
	CHECK(0 != blockOffset);

	lastBlockOffset = blockOffset;
	/* scan chain */
	while (0 != blockOffset) {
		lastBlockOffset = blockOffset;
		
		/* read the offset to the next block from disk */
		CHECK(pfread(ht->keyFP, blockOffset + KEY_FILE_BLOCK_SIZE(ht) - sizeof(DEHT_DISK_PTR), (byte_t *) &blockOffset, sizeof(blockOffset)));
	}

	CHECK(0 != lastBlockOffset);

	goto LBL_CLEANUP;

LBL_ERROR:
	lastBlockOffset = 0;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return lastBlockOffset;
}



DEHT_DISK_PTR DEHT_findLastBlockForBucket(DEHT * ht, ulong_t bucketIndex)
{
	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(bucketIndex < ht->header.numEntriesInHashTable);

	if ((NULL == ht->hashPointersForLastBlockImageInMemory) ||
	    (0 == ht->hashPointersForLastBlockImageInMemory[bucketIndex]) ) {
		TRACE("scanning chain");

		return DEHT_findLastBlockForBucketDumb(ht, bucketIndex);
	}
	else {
		TRACE("using cache");
		return ht->hashPointersForLastBlockImageInMemory[bucketIndex];
	}

LBL_ERROR:
	TRACE_FUNC_ERROR();
	return 0;


}

bool_t DEHT_allocEmptyLocationInBucket(DEHT * ht, ulong_t bucketIndex,
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

	*blockDiskPtr = DEHT_findLastBlockForBucket(ht, bucketIndex);
	CHECK(0 != *blockDiskPtr);

	/* read it */
	CHECK(pfread(ht->keyFP, *blockDiskPtr, blockDataOut, KEY_FILE_BLOCK_SIZE(ht)));

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
		
		CHECK(pfwrite(ht->keyFP, *blockDiskPtr, blockDataOut, KEY_FILE_BLOCK_SIZE(ht)));
		
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


DEHT_DISK_PTR DEHT_allocKeyBlock(DEHT * ht)
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

	goto LBL_CLEANUP;

LBL_ERROR:
	newBlock = 0;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return newBlock;
}


bool_t DEHT_readDataAtOffset(DEHT * ht, DEHT_DISK_PTR dataBlockOffset, 
			     byte_t * data, ulong_t dataMaxAllowedLength, ulong_t * bytesRead)
{
	bool_t ret = FALSE;

	byte_t dataLen = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != data);
	CHECK(NULL != bytesRead);
	*bytesRead = 0;

	CHECK(pfread(ht->dataFP, dataBlockOffset, &dataLen, sizeof(dataLen)));

	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): data size is %d\n", __FILE__, __LINE__, __FUNCTION__, dataLen));

	*bytesRead = fread(data, 1, dataMaxAllowedLength, ht->dataFP);
	CHECK(0 < *bytesRead);

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



bool_t DEHT_writeData(DEHT * ht, const byte_t * data, ulong_t dataLen, 
		      DEHT_DISK_PTR * newDataOffset)
{
	bool_t ret = FALSE;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != data);
	CHECK(NULL != newDataOffset);
	*newDataOffset = 0;

	(void) fflush(ht->keyFP);
	CHECK(0 == fseek(ht->dataFP, 0, SEEK_END));
	*newDataOffset = ftell(ht->dataFP);

	CHECK(1 == fwrite(&dataLen, sizeof(byte_t), 1, ht->dataFP));
	CHECK(1 == fwrite(data, dataLen, 1, ht->dataFP));

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
       

void DEHT_freeResources(DEHT * ht, bool_t removeFiles)
{
	TRACE_FUNC_ENTRY();

	CHECK (NULL != ht);

	FCLOSE(ht->keyFP);
	FCLOSE(ht->dataFP);

	/* free ht cache if present */
	FREE(ht->hashTableOfPointersImageInMemory);
	FREE(ht->hashPointersForLastBlockImageInMemory);

	if (removeFiles) {
		/* attempt to remove bad files. Errors are silenced */
		(void) remove(ht->sKeyfileName);
		(void) remove(ht->sDatafileName);
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



