
#include "DEHT.h"
#include "types.h"
#include "utils.h"




DEHT *create_empty_DEHT(const char *prefix,
                        hashKeyIntoTableFunctionPtr hashfun, hashKeyforEfficientComparisonFunctionPtr validfun,
                        const char *dictName,
                        int numEntriesInHashTable, int nPairsPerBlock, int nBytesPerKey)
{
	bool_t errorState = FALSE;

	DEHT * instance = NULL;

	TRACE_FUNC_ENTRY();

	/* sanity */
	CHECK(0 != numEntriesInHashTable);
	CHECK(0 != nPairsPerBlock);
	CHECK(0 != nBytesPerKey);


	instance = DEHT_init_instance(prefix, "w+b", "w+b", hashfun, validfun);
	CHECK(NULL != instance);

	/* Do extra inits */
	SAFE_STRNCPY(instance->header.sDictionaryName, dictName, sizeof(instance->header.sDictionaryName));
	instance->header.numEntriesInHashTable = numEntriesInHashTable;
	instance->header.nPairsPerBlock = nPairsPerBlock;
	instance->header.nBytesPerValidationKey = nBytesPerKey;
	instance->header.numUnrelatedBytesSaved = 0;

	/* write header to disk */
	CHECK(1 == fwrite(&(instance->header), sizeof(instance->header), 1, instance->keyFP));
	fflush(instance->keyFP);

	/* alloc pointer table */
	instance->hashTableOfPointersImageInMemory = malloc(KEY_FILE_BUCKET_POINTERS_SIZE(instance));
	CHECK(NULL != instance->hashTableOfPointersImageInMemory);
	memset(instance->hashTableOfPointersImageInMemory, 0, KEY_FILE_BUCKET_POINTERS_SIZE(instance));

	/* write pointer table to disk */
	CHECK(pfwrite(instance->keyFP, sizeof(instance->header), instance->hashTableOfPointersImageInMemory, KEY_FILE_BUCKET_POINTERS_SIZE(instance)));
	

	goto LBL_CLEANUP;	

LBL_ERROR:
	errorState = TRUE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	if (errorState) {
		if (NULL != instance) {
			DEHT_freeResources(instance);
		}
	}
	else {
		return instance;
	}
}


DEHT * load_DEHT_from_files(const char *prefix, hashKeyIntoTableFunctionPtr hashfun, hashKeyforEfficientComparisonFunctionPtr validfun)
{
	bool_t errorState = FALSE;

	DEHT * instance = NULL;

	TRACE_FUNC_ENTRY();

	instance = DEHT_init_instance(prefix, "r+b", "r+b", hashfun, validfun);
	CHECK(NULL != instance);


	/* load dict settings from file */
	CHECK(1 == fread(&(instance->header), sizeof(instance->header), 1, instance->keyFP));

	goto LBL_CLEANUP;	

LBL_ERROR:
	errorState = TRUE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	if (errorState) {
		if (NULL != instance) {
			DEHT_freeResources(instance);
		}
	}
	else {
		return instance;
	}
}




DEHT * DEHT_init_instance (const char * prefix, char * keyFileMode, char * dataFileMode, 
			   hashKeyIntoTableFunctionPtr hashfun, hashKeyforEfficientComparisonFunctionPtr validfun)
{
	bool_t errorState = TRUE;
	DEHT * instance = NULL;

	TRACE_FUNC_ENTRY();

	/* sanity */
	CHECK(NULL != prefix);
	CHECK(NULL != keyFileMode);
	CHECK(NULL != dataFileMode);
	CHECK(NULL != hashfun);
	CHECK(NULL != validfun);


	instance = malloc(sizeof(DEHT));
	CHECK(NULL != instance);

	memset(instance, 0, sizeof(DEHT));

	SAFE_STRNCPY(instance->sKeyfileName, prefix, sizeof(instance->sKeyfileName));
	SAFE_STRNCAT(instance->sKeyfileName, KEY_FILE_EXT, sizeof(instance->sKeyfileName));

	SAFE_STRNCPY(instance->sDatafileName, prefix, sizeof(instance->sKeyfileName));
	SAFE_STRNCAT(instance->sDatafileName, DATA_FILE_EXT, sizeof(instance->sKeyfileName));

	instance->keyFP = fopen(instance->sKeyfileName, keyFileMode);
	CHECK(NULL != instance->keyFP);

	instance->dataFP = fopen(instance->sDatafileName, dataFileMode);
	CHECK(NULL != instance->dataFP);	
	
	instance->hashTableOfPointersImageInMemory = NULL;

	errorState = FALSE;
	goto LBL_CLEANUP;


LBL_ERROR:
	errorState = TRUE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	if (errorState) {
		FREE(instance);
		
		return NULL;
	}
	else {
		return instance;
	}
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
	

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != key);
	CHECK(NULL != data);

	/* allocate a buffer for DEHT_query_internal */
	tempKeyBlock = malloc(KEY_FILE_BLOCK_SIZE(ht));
	CHECK(NULL != tempKeyBlock);
	
	ret = DEHT_query_internal(ht, key, keyLength, data, dataLength, tempKeyBlock, KEY_FILE_BLOCK_SIZE(ht), 
				  &keyBlockDiskOffset, &keyIndex, &lastKeyBlockDiskOffset);

	switch(ret) {
	case DEHT_STATUS_NOT_NEEDED:
		TRACE("simple insert");
		/* not found - just insert */
		CHECK(DEHT_STATUS_SUCCESS == add_DEHT (ht, key, keyLength, data, dataLength));

		ret = DEHT_STATUS_SUCCESS;

		break;

	case DEHT_STATUS_SUCCESS:
		TRACE("re-writing record");
		/* found - need to update */

		/* write the new data to the data file */
		CHECK(DEHT_writeData(ht, data, dataLength, &newDataOffset));

		/* update the target record */
		targetRecord = GET_N_REC_PTR_IN_BLOCK(ht, tempKeyBlock, keyIndex);
		targetRecord->dataOffset = newDataOffset;

		/* update block on disk */
		CHECK(pfwrite(ht->keyFP, keyBlockDiskOffset, tempKeyBlock, KEY_FILE_BLOCK_SIZE(ht)));

		ret = DEHT_STATUS_NOT_NEEDED;
		
		break;

	default:
	case DEHT_STATUS_FAIL:
		/* internal error */
		goto LBL_ERROR;
	}

	goto LBL_CLEANUP;	

LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	FREE(tempKeyBlock);

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
	hashTableIndex = ht->hashFunc(key, keyLength, ht->header.numEntriesInHashTable);
	TRACE_FPRINTF(stderr, "%s: bucket index=%#x\n", __FUNCTION__, hashTableIndex);

	blockContent = malloc(KEY_FILE_BLOCK_SIZE(ht));
	CHECK(NULL != blockContent);
	CHECK(DEHT_findEmptyLocationInBucket(ht, hashTableIndex, blockContent, KEY_FILE_BLOCK_SIZE(ht),
					     &keyBlockOffset, &freeIndex));

	TRACE_FPRINTF(stderr, "%s: using block at %#x, index=%lu\n", __FUNCTION__, (uint_t) keyBlockOffset, freeIndex);

	targetRec = GET_N_REC_PTR_IN_BLOCK(ht, blockContent, freeIndex);

	/* calc validation key and fill in record */
	/*! Note: return value isn't checked since the spec failed to include details regarding the key 
		  validation function interface */
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

	/* allocate a buffer for DEHT_query_internal */
	tempKeyBlock = malloc(KEY_FILE_BLOCK_SIZE(ht));
	CHECK(NULL != tempKeyBlock);
	
	ret = DEHT_query_internal(ht, key, keyLength, data, dataMaxAllowedLength, tempKeyBlock, KEY_FILE_BLOCK_SIZE(ht), 
				  &keyBlockDiskOffset, &keyIndex, &lastKeyBlockDiskOffset);

	goto LBL_CLEANUP;	

LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	FREE(tempKeyBlock);

	return ret;
}





int DEHT_query_internal(DEHT *ht, const unsigned char *key, int keyLength, const unsigned char *data, int dataMaxAllowedLength,
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
	

	/* calc hash for key */
	hashTableIndex = ht->hashFunc(key, keyLength, ht->header.numEntriesInHashTable);
	TRACE_FPRINTF(stderr, "%s: bucket index=%#x\n", __FUNCTION__, hashTableIndex);

	if (NULL != ht->hashTableOfPointersImageInMemory) {
		*keyBlockDiskOffset = ht->hashTableOfPointersImageInMemory[hashTableIndex];
		TRACE_FPRINTF(stderr, "%s: first ptr from cache: %#x\n", __FUNCTION__, (uint_t) *keyBlockDiskOffset);
	}
	else {
		/* no cache - read from disk */
		CHECK(pfread(ht->keyFP, sizeof(ht->header) + hashTableIndex * sizeof(DEHT_DISK_PTR), keyBlockDiskOffset, sizeof(*keyBlockDiskOffset)));
		TRACE_FPRINTF(stderr, "%s: first ptr from disk: %#x\n", __FUNCTION__, (uint_t) *keyBlockDiskOffset);
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
		TRACE_FPRINTF(stderr, "%s: next ptr from disk: %#x\n", __FUNCTION__, (uint_t) *keyBlockDiskOffset);

		/*! TODO: update last block cache if present !*/
	}

	dataBlockOffset = 0;
	if (*keyIndex < ht->header.nPairsPerBlock) {
		dataBlockOffset = currPair->dataOffset;
	}
	else {
		/* we scanned everything, but found no matching key */
		ret = DEHT_STATUS_NOT_NEEDED;
		goto LBL_CLEANUP;
	}

	bytesRead = 0;
	if (DEHT_readDataAtOffset(ht, dataBlockOffset, (byte_t *) data, dataMaxAllowedLength, &bytesRead)) {
		ret = bytesRead;
		goto LBL_CLEANUP;
	}
	else {
		goto LBL_ERROR;	
	}
	
LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	FREE(validationKey);
	return ret;

}







int read_DEHT_pointers_table(DEHT *ht)
{
	int ret = DEHT_STATUS_FAIL;
	size_t rawTableSize = 0;
	int oldFp = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);

	if (NULL != ht->hashTableOfPointersImageInMemory) {
		return DEHT_STATUS_NOT_NEEDED;
	}

	/* alloc cache */
	ht->hashTableOfPointersImageInMemory = malloc(KEY_FILE_BUCKET_POINTERS_SIZE(ht));
	CHECK(NULL != ht->hashTableOfPointersImageInMemory);

	/* read the offset table */
	CHECK(pfread(ht->keyFP, sizeof(ht->header), ht->hashTableOfPointersImageInMemory, KEY_FILE_BUCKET_POINTERS_SIZE(ht)));

	ret = DEHT_STATUS_SUCCESS;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	return ret;
}

int write_DEHT_pointers_table(DEHT *ht)
{
	int ret = DEHT_STATUS_FAIL;
	int oldFp = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);

	if (NULL == ht->hashTableOfPointersImageInMemory) {
		return DEHT_STATUS_NOT_NEEDED;
	}

	/* write the offset table */
	CHECK(pfwrite(ht->keyFP, sizeof(ht->header), ht->hashTableOfPointersImageInMemory, KEY_FILE_BUCKET_POINTERS_SIZE(ht)));

	ret = DEHT_STATUS_SUCCESS;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = DEHT_STATUS_FAIL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
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
	return ret;
}


DEHT_DISK_PTR DEHT_findLastBlockForBucketDumb(DEHT * ht, ulong_t bucketIndex)
{
	DEHT_DISK_PTR blockOffset = 0;
	DEHT_DISK_PTR lastBlockOffset = 0;
	
	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(DEHT_STATUS_FAIL != read_DEHT_pointers_table(ht));


	blockOffset = ht->hashTableOfPointersImageInMemory[bucketIndex];
	lastBlockOffset = blockOffset;
	while (0 != blockOffset) {
		lastBlockOffset = blockOffset;
		
		/* read the offset to the next block from disk */
		CHECK(pfread(ht->keyFP, blockOffset + KEY_FILE_BLOCK_SIZE(ht) - sizeof(DEHT_DISK_PTR), &blockOffset, sizeof(blockOffset)));

	}

	if (0 == lastBlockOffset) {
		/* this is the first block - use the end of the file */
		CHECK(0 == fseek(ht->keyFP, SEEK_END, 0));
		lastBlockOffset = ftell(ht->keyFP);
		
		/* write an empty block */
		
		/* assign it to the bucket */		
		ht->hashTableOfPointersImageInMemory[bucketIndex] = lastBlockOffset;
	}

	/* Update cache if present */
	if (NULL != ht->hashPointersForLastBlockImageInMemory) {
		ht->hashPointersForLastBlockImageInMemory[bucketIndex] = lastBlockOffset;
	}

	goto LBL_CLEANUP;

LBL_ERROR:
	lastBlockOffset = 0;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	return lastBlockOffset;
}



DEHT_DISK_PTR DEHT_findLastBlockForBucket(DEHT * ht, ulong_t bucketIndex)
{
	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);

	if (NULL != ht->hashPointersForLastBlockImageInMemory) {
		return ht->hashPointersForLastBlockImageInMemory[bucketIndex];
	}
	else {
		return DEHT_findLastBlockForBucketDumb(ht, bucketIndex);
	}

LBL_ERROR:
	TRACE_FUNC_ERROR();
	return 0;

}

bool_t DEHT_findEmptyLocationInBucket(DEHT * ht, ulong_t bucketIndex,
					     byte_t * blockDataOut, ulong_t blockDataLen,
					     DEHT_DISK_PTR * blockDiskPtr, ulong_t * firstFreeIndex)
{
	bool_t ret = FALSE;

	DEHT_DISK_PTR newBlock = 0;

	byte_t * emptyPair = NULL;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != blockDataOut);
	CHECK(KEY_FILE_BLOCK_SIZE(ht) <= blockDataLen);
	CHECK(NULL != blockDiskPtr);
	CHECK(NULL != firstFreeIndex);



	*blockDiskPtr = DEHT_findLastBlockForBucket(ht, bucketIndex);

	/* read it */
	CHECK(pfread(ht->keyFP, *blockDiskPtr, blockDataOut, KEY_FILE_BLOCK_SIZE(ht)));

	/* alloc a dummy empty key-data pair */
	emptyPair = malloc(KEY_FILE_RECORD_SIZE(ht));
	CHECK(NULL != emptyPair);
	memset(emptyPair, 0, KEY_FILE_RECORD_SIZE(ht));

	/* scan the block, looking for an empty pair */
	for (*firstFreeIndex = 0;   *firstFreeIndex < ht->header.nPairsPerBlock;  ++*firstFreeIndex) {
		if (0 == memcmp(emptyPair, blockDataOut + *firstFreeIndex * KEY_FILE_RECORD_SIZE(ht), KEY_FILE_RECORD_SIZE(ht))) {
			/*! TODO: stricter checks / add block filled count !*/
			break;
		}
	}
	
	/* see if we've reached the end of the block */
	if (*firstFreeIndex >= ht->header.nPairsPerBlock) {
		newBlock = DEHT_allocKeyBlock(ht);
		CHECK(0 != newBlock);

		/* update old block */
		SET_NEXT_BLOCK_PTR(ht, blockDataOut, newBlock);

		CHECK(pfwrite(ht->keyFP, *blockDiskPtr, blockDataOut, KEY_FILE_BLOCK_SIZE(ht)));
		
		/* now, just return the reference to the start of the new block */
		memset(blockDataOut, 0, KEY_FILE_BLOCK_SIZE(ht));
		*blockDiskPtr = newBlock;
		*firstFreeIndex = 0;
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	*blockDiskPtr = 0;
	*firstFreeIndex = 0;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
		
	return ret;
}


DEHT_DISK_PTR DEHT_allocKeyBlock(DEHT * ht)
{
	DEHT_DISK_PTR newBlock  = 0;
	DEHT_DISK_PTR * tempBlockContent = NULL;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);

	/* this is the first block - use the end of the file */
	CHECK(0 == fseek(ht->keyFP, SEEK_END, 0));
	newBlock = ftell(ht->keyFP);
	
	/* alloc an empty block (init to NULLs) */
	tempBlockContent = malloc(KEY_FILE_BLOCK_SIZE(ht));
	CHECK(NULL != tempBlockContent);
	memset(tempBlockContent, 0, KEY_FILE_BLOCK_SIZE(ht));

	/* write to disk */
	CHECK(1 == fwrite(tempBlockContent, KEY_FILE_BLOCK_SIZE(ht), 1, ht->keyFP));

	goto LBL_CLEANUP;

LBL_ERROR:
	newBlock = 0;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	FREE(tempBlockContent);

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

	TRACE_FPRINTF(stderr, "%s: key size is %d\n", __FUNCTION__, dataLen);

	*bytesRead = fread(data, 1, dataMaxAllowedLength, ht->dataFP);
	CHECK(0 < *bytesRead);

	ret = TRUE;
	goto LBL_CLEANUP;
	
LBL_ERROR:
	*bytesRead = 0;
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
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

	CHECK(0 == fseek(ht->dataFP, SEEK_END, 0));
	*newDataOffset = ftell(ht->dataFP);
	CHECK(1 == fwrite(data, dataLen, 1, ht->dataFP));

	ret = TRUE;
	goto LBL_CLEANUP;
	
LBL_ERROR:
	*newDataOffset = 0;
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	return ret;

}



void lock_DEHT_files(DEHT *ht)
{

	TRACE_FUNC_ENTRY();

	/* If present, serialize pointer table to disk. Errors are ignored -
	   we try to do as much as we can in spite of error for robustness */
	(void) write_DEHT_pointers_table(ht);

	fflush(ht->keyFP);
	fflush(ht->dataFP);

	DEHT_freeResources(ht);
}
       

void DEHT_freeResources(DEHT * instance)
{
	TRACE_FUNC_ENTRY();

	CHECK (NULL != instance);

	FCLOSE(instance->keyFP);
	FCLOSE(instance->dataFP);

	/* free ht cache if present */
	FREE(instance->hashTableOfPointersImageInMemory);
	FREE(instance->hashPointersForLastBlockImageInMemory);
	FREE(instance);

	goto LBL_CLEANUP;

LBL_ERROR:
	/* do nothing special - just quit */
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	/* bye */
	return;
}



