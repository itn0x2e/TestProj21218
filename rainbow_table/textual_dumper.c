

static bool_t printSeeds(RainbowTable_t * rt, FILE * fd)
{
	bool_t ret = FALSE;

	ulong_t userBytesSize = 0;
	ulong_t seedIndex = 0;

	char seedStr[sizeof(RainbowSeed_t) * 2 + 1];


	TRACE_FUNC_ENTRY();


	CHECK(NULL != rt);
	CHECK(NULL != fd);

	/* load user bytes buffer */
	CHECK(DEHT_STATUS_FAIL != DEHT_readUserBytes(rt.hashTable, (void **) &(rt.config), &userBytesSize));
	CHECK(NULL != rt.config);
	CHECK(userBytesSize == sizeof(RainbowTableConfig_t) + rt.config->chainLength * sizeof(RainbowSeed_t));


	fprintf(fd, "Seeds:\n");
	for (seedIndex = 0; seedIndex < rt.config->chainLength;  ++seedIndex) {
		binary2hexa((byte_t *) (rt.config->seeds + seedIndex), sizeof(RainbowSeed_t), seedStr, sizeof(seedStr));
		fprintf(fd, "%s\n", seedStr);
	}

	fprintf(fd, "\n");

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}




static bool_t printPasswordsAndChainsInBlock(RainbowTable_t * rt, 
					    FILE * file1, FILE * file2, 
					    byte_t * currKeyBlock)
{
	byte_t currPassword[MAX_PASSWORD_LEN];
	ulong_t keyIndex = 0;

	KeyFilePair_t currKeyPair = NULL;
	ulong_t bytesRead = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != rt);
	CHECK(NULL != fd);
	CHECK(NULL != currKeyBlock);

	for (keyIndex = 0;  keyIndex < GET_USED_RECORD_COUNT(currKeyBlock); ++keyIndex) {
		currKeyPair = GET_N_REC_PTR_IN_BLOCK(rt->hashTable, currKeyBlock, keyIndex);
		
		CHECK(DEHT_readDataAtOffset(rt->hashTable, currKeyPair->dataOffset,
					    currPassword, sizeof(currPassword) - 1, &bytesRead);
		/* terminate */
		currPassword[MIN(bytesRead, sizeof(currPassword) - 1] = 0x00;

		fprintf(fd, "%s\n", currPassword);
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


static bool_t printPasswordsAndChains(RainbowTable_t * rt, 
				      FILE * file1, FILE * file2)
{
{
	bool_t ret = FALSE;

	ulong_t userBytesSize = 0;
	ulong_t seedIndex = 0;

	char passwordStr[MAX_PASSWORD_LEN * 2 + 1];

	byte_t * currBlock = NULL;
	DEHT_DISK_PTR currBlockDiskOffset = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != rt);
	CHECK(NULL != fd);

	/* alloc a block for scanning the DEHT */
	currKeyBlock = malloc(KEY_FILE_BLOCK_SIZE(rt.hashTable));
	CHECK(NULL != currKeyBlock);

	fprintf(fd, "Passwords:\n");

	/* Dump passwords, guided by the key file. We use this method rather than directly pulling info from the data file, there may have been
	   collision, causing some data in data file to no longer be relevant */
	/* scan each bucket */
	for (bucketId = 0;  bucketId <  rt.hashTable->header.numEntriesInHashTable; ++bucketId) {
		/* read the first block */
		currBlockDiskOffset = DEHT_findFirstBlockForBucket(rt.hashTable, bucketId);
		if (0 == currBlockDiskOffset) {
			TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): bucket %lu is empty\n", __FILE__, __LINE__, __FUNCTION__, bucketId));
			continue;
		}

		TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): passwords for bucket %lu:\n", __FILE__, __LINE__, __FUNCTION__, bucketId));

		/* walk the block linked list */
		while(0 != currBlockDiskOffset) {
			/* read block to memory */
			CHECK(pfread(rt->hashTable.keyFP, currBlockDiskOffset, currKeyBlock, KEY_FILE_BLOCK_SIZE(rt->hashTable)));

			/* dump passwords */
			CHECK(printPasswordsInBlock(rt, fd, currKeyBlock));

			/* move to next block */
			currBlockDiskOffset = GET_NEXT_BLOCK_PTR(rt->hashTable, currKeyBlock);
		}
	}


	fprintf(fd, "\n");

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}


bool_t RT_print(FILE * file1, FILE * file2,

		const char * hashTableFilePrefix,
		bool_t enableFirstBlockCache,
		bool_t enableLastBlockCache)
{
	RainbowTable_t rt;


	ulong_t bucketId = 0;
	


	TRACE_FUNC_ENTRY();

	CHECK(NULL != hashTableFilePrefix);
	CHECK(NULL != file1);
	CHECK(NULL != file2);


	/* init */
	memset(&rt, 0, sizeof(RainbowTable_t));

	rt.hashTable = load_DEHT_from_files(hashTableFilePrefix, 
			       DEHT_keyToTableIndexHasher, DEHT_keyToValidationKeyHasher64);
	CHECK(NULL != rt.hashTable);

	if (enableFirstBlockCache) {
		CHECK(DEHT_STATUS_FAIL != read_DEHT_pointers_table(rt.hashTable));
	}
	if (enableLastBlockCache) {
		CHECK(DEHT_STATUS_FAIL != calc_DEHT_last_block_per_bucket(rt.hashTable));
	}

	/* find the hash type */
	rt.hashFunc = getHashFunFromName(rt.hashTable->header.sDictionaryName);
	CHECK(NULL != rt.hashFunc);

	/* load user bytes buffer */
	CHECK(DEHT_STATUS_FAIL != DEHT_readUserBytes(rt.hashTable, (void **) &(rt.config), &userBytesSize));
	CHECK(NULL != rt.config);
	CHECK(userBytesSize == sizeof(RainbowTableConfig_t) + rt.config->chainLength * sizeof(RainbowSeed_t));

	CHECK(printSeeds(&rt, file1));
	CHECK(printPasswordsAndChains(&rt, file1, file2));










	ret = self;
	goto LBL_CLEANUP;
	
	
LBL_ERROR:
	ret = NULL;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	if (NULL == ret) {
		if (NULL != self) {
			RT_close(self);
			self = NULL;
		}
	}

	TRACE_FUNC_EXIT();

	return ret;

}
