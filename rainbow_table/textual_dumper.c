

#include "../common/constants.h"
#include "../common/utils.h"
#include "../common/misc.h"
#include "../common/io.h"
#include "../common/types.h"
#include "../DEHT/DEHT.h"
#include "../DEHT/hash_funcs.h"
#include "rainbow_table.h"

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
	CHECK(DEHT_STATUS_FAIL != DEHT_readUserBytes(rt->hashTable, (void **) &(rt->config), &userBytesSize));
	CHECK(NULL != rt->config);
	CHECK(userBytesSize == sizeof(RainbowTableConfig_t) + rt->config->chainLength * sizeof(RainbowSeed_t));


	fprintf(fd, "Seeds:\n");
	for (seedIndex = 0; seedIndex < rt->config->chainLength;  ++seedIndex) {
		binary2hexa((byte_t *) (rt->config->seeds + seedIndex), sizeof(RainbowSeed_t), seedStr, sizeof(seedStr));
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


bool_t printAndVerifyChain(RainbowTable_t * rt, FILE * outputFD,
			   char * password, ulong_t passwordLen,
			   byte_t * hashOut, ulong_t hashOutLen)
{
	bool_t ret = FALSE;

	byte_t verifiedHash[MAX_DIGEST_LEN];

	TRACE_FUNC_ENTRY();	

	CHECK(NULL != rt);
	CHECK(NULL != password);

	/* scan the chain and print in the process */
	CHECK(buildChain(FALSE,
			rt->config->seeds, rt->config->chainLength,
			rt->hashFunc,
			rt->passGenerator, (byte_t *) rt->generatorPassword,
			(byte_t *) password, passwordLen,
			verifiedHash, sizeof(verifiedHash),
			NULL, 0,
			outputFD));


	/* copy result hash to user */
	memcpy(hashOut, verifiedHash, MIN(sizeof(verifiedHash), hashOutLen));

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
					     byte_t * currKeyBlock, ulong_t bucketId, 
					     FILE * file1, FILE * file2)
{

	bool_t ret = FALSE;

	ulong_t keyIndex = 0;

	KeyFilePair_t * currKeyPair = NULL;

	byte_t currPassword[MAX_PASSWORD_LEN];
	ulong_t bytesRead = 0;

	byte_t chainHash[MAX_DIGEST_LEN];
	ulong_t hashLen;

	int chainHashTableIndex = 0;
	byte_t chainValidationKey[8];
	

	TRACE_FUNC_ENTRY();

	CHECK(NULL != rt);
	CHECK(NULL != file1);
	CHECK(NULL != file2);
	CHECK(NULL != currKeyBlock);

	hashLen = getHashFunDigestLength(rt->hashFunc);
	CHECK(0 != hashLen);

	for (keyIndex = 0;  keyIndex < GET_USED_RECORD_COUNT(currKeyBlock); ++keyIndex) {
		currKeyPair = GET_N_REC_PTR_IN_BLOCK(rt->hashTable, currKeyBlock, keyIndex);
		
		CHECK(DEHT_readDataAtOffset(rt->hashTable, currKeyPair->dataOffset,
					    currPassword, sizeof(currPassword) - 1, &bytesRead));
		/* terminate */
		currPassword[MIN(bytesRead, sizeof(currPassword) - 1)] = 0x00;

		fprintf(file1, "%s\n", currPassword);

		/* keep going even if we encounter an error on a single password */
		if (!printAndVerifyChain(rt, file2,
					   (char *) currPassword, bytesRead,
					   chainHash, sizeof(chainHash))) {
			continue;
		}

		/* Compute the two sub-indices created based on this hash */
		chainHashTableIndex = DEHT_keyToTableIndexHasher(chainHash, hashLen, rt->hashTable->header.numEntriesInHashTable);
		(void) DEHT_keyToValidationKeyHasher64(chainHash, hashLen, (byte_t *) &chainValidationKey);

		/* compare to the current pair state */
		if ((bucketId != chainHashTableIndex) || (0 != memcmp(chainValidationKey, currKeyPair->key, sizeof(chainValidationKey)))) {
			fprintf(stderr, "Error: when begin with %s get wrong chainn\n", currPassword);

			/* (fail silently here, to keep testing all the entries in the table */
		}

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
	bool_t ret = FALSE;

	ulong_t bucketId = 0;

	byte_t * currKeyBlock = NULL;
	DEHT_DISK_PTR currBlockDiskOffset = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != rt);
	CHECK(NULL != file1);
	CHECK(NULL != file2);

	/* alloc a block for scanning the DEHT */
	currKeyBlock = malloc(KEY_FILE_BLOCK_SIZE(rt->hashTable));
	CHECK(NULL != currKeyBlock);

	fprintf(file1, "Passwords:\n");

	/* Dump passwords, guided by the key file. We use this method rather than directly pulling info from the data file, there may have been
	   collision, causing some data in data file to no longer be relevant */
	/* scan each bucket */
	for (bucketId = 0;  bucketId <  rt->hashTable->header.numEntriesInHashTable; ++bucketId) {
		/* read the first block */
		currBlockDiskOffset = DEHT_findFirstBlockForBucket(rt->hashTable, bucketId);
		if (0 == currBlockDiskOffset) {
			TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): bucket %lu is empty\n", __FILE__, __LINE__, __FUNCTION__, bucketId));
			continue;
		}

		TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): passwords for bucket %lu:\n", __FILE__, __LINE__, __FUNCTION__, bucketId));

		/* walk the block linked list */
		while(0 != currBlockDiskOffset) {
			/* read block to memory */
			CHECK(pfread(rt->hashTable->keyFP, currBlockDiskOffset, currKeyBlock, KEY_FILE_BLOCK_SIZE(rt->hashTable)));

			/* dump passwords */
			CHECK(printPasswordsAndChainsInBlock(rt, 
								currKeyBlock, bucketId,
								file1, file2));

			/* move to next block */
			currBlockDiskOffset = GET_NEXT_BLOCK_PTR(rt->hashTable, currKeyBlock);
		}
	}


	fprintf(file1, "\n");

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:

	FREE(currKeyBlock);
	
	TRACE_FUNC_EXIT();
	return ret;
}


bool_t RT_print(FILE * file1, FILE * file2,

		const passwordGenerator_t * passGenerator,
		char * generatorPassword,

		const char * hashTableFilePrefix)
{
	bool_t ret = FALSE;

	RainbowTable_t * rt = NULL;



	TRACE_FUNC_ENTRY();

	CHECK(NULL != hashTableFilePrefix);
	CHECK(NULL != file1);
	CHECK(NULL != file2);


	rt = RT_open(passGenerator, generatorPassword, 
		     hashTableFilePrefix);
	CHECK(NULL != rt);

	CHECK(printSeeds(rt, file1));
	CHECK(printPasswordsAndChains(rt, file1, file2));

	ret = TRUE;
	goto LBL_CLEANUP;
	
	
LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:

	if (NULL != rt) {
		RT_close(rt);
		rt = NULL;
	}

	TRACE_FUNC_EXIT();

	return ret;
}
