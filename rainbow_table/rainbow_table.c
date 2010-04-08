
#include "rainbow_table.h"
#include "../common/constants.h"
#include "../common/utils.h"
#include "../common/types.h"
#include "../common/misc.h"
#include "../password/password_enumerator.h"
#include "../password/password_generator.h"
#include "../DEHT/DEHT.h"
#include "../DEHT/hash_funcs.h"
#include "../common/rand_utils.h"


bool_t buildChain(RainbowTableConfig_t * rainbowConfig, 
			 BasicHashFunctionPtr hashFunc,

			 byte_t * firstPassword, int firstPasswordLen,
			 const passwordGenerator_t * passGenerator, byte_t * generatorPassword,

			 byte_t * resultHash, ulong_t resultHashLen)
{
	bool_t ret = FALSE;

	ulong_t inChainIndex = 0;

	byte_t currHash[MAX_DIGEST_LEN];
	ulong_t hashLen = 0;

	/* "k" */
	ulong_t nextPasswordIndex = 0;


	TRACE_FUNC_ENTRY();

	CHECK(NULL != rainbowConfig);
	CHECK(NULL != firstPassword);
	CHECK(NULL != passGenerator);
	CHECK(NULL != generatorPassword);
	CHECK(NULL != resultHash);


	/* calc first hash */
	hashLen = hashFunc(firstPassword, firstPasswordLen, currHash);
	CHECK(0 != hashLen);
	CHECK(hashLen <= resultHashLen);

	/* perform the steps detailed in the project specification to compute the chain for this start point */
	for (inChainIndex = 0; inChainIndex < rainbowConfig->chainLength; ++inChainIndex) {

		/* PRNG using the current hash and the seed corresponding to the current inChainIndex */
		CHECK(0 != miniHash((byte_t *) &nextPasswordIndex, sizeof(nextPasswordIndex),
				    (byte_t *) (rainbowConfig->seeds + inChainIndex), sizeof(rainbowConfig->seeds[0]), 
				    currHash, hashLen));

		/* wrap-around the password space size */
		nextPasswordIndex = nextPasswordIndex % passwordGeneratorGetSize(passGenerator);

		/* generate k-randomized password */
		passwordGeneratorCalculatePassword(passGenerator, nextPasswordIndex, (char *) generatorPassword);
		

		/* calc next hash */
		CHECK(0 != hashFunc(generatorPassword, strlen((char *) generatorPassword), currHash));
	}

	memcpy(resultHash, currHash, resultHashLen);

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}


bool_t generateRainbowTable(
			passwordEnumerator_t * passEnumerator,
			const passwordGenerator_t * passGenerator,

			char * enumeratorPassword,
			char * generatorPassword,

			BasicHashFunctionPtr hashFunc,

			ulong_t rainbowChainLen,

			const char * hashTableFilePrefix,
			ulong_t nHashTableEntries,
			ulong_t nPairsPerBlock,
			bool_t enableFirstBlockCache,
			bool_t enableLastBlockCache)
{
	bool_t ret = FALSE;

	DEHT * ht = NULL;

	ulong_t chainIndex = 0;

	RainbowTableConfig_t * rainbowConfig = NULL;
	ulong_t userBytesSize = 0;
	ulong_t chainHashLen = 0;

	byte_t chainHash[MAX_DIGEST_LEN];


	TRACE_FUNC_ENTRY();

	CHECK(NULL != passEnumerator);
	CHECK(NULL != passGenerator);
	CHECK(NULL != enumeratorPassword);
	CHECK(NULL != generatorPassword);
	CHECK(NULL != hashFunc);
	CHECK(NULL != hashTableFilePrefix);

	ht = create_empty_DEHT(hashTableFilePrefix, 
			       DEHT_keyToTableIndexHasher, DEHT_keyToValidationKeyHasher64, 
			       getNameFromHashFun(hashFunc), 
			       nHashTableEntries, nPairsPerBlock, sizeof(ulong_t), sizeof(RainbowTableConfig_t) + sizeof(RainbowSeed_t) * rainbowChainLen);
	CHECK(NULL != ht);

	if (enableFirstBlockCache) {
		CHECK(DEHT_STATUS_FAIL != read_DEHT_pointers_table(ht));
	}
	if (enableLastBlockCache) {
		CHECK(DEHT_STATUS_FAIL != calc_DEHT_last_block_per_bucket(ht));
	}


	/* load user bytes buffer (even though it should be empty right now, this will allow us to use the table's buffer, which gets saved to disk automagically) */
	CHECK(DEHT_STATUS_FAIL != DEHT_readUserBytes(ht, (void **) &rainbowConfig, &userBytesSize));
	CHECK(NULL != rainbowConfig);
	CHECK(userBytesSize == sizeof(RainbowTableConfig_t) + rainbowChainLen * sizeof(RainbowSeed_t));

	/* store chain len */
	rainbowConfig->chainLength = rainbowChainLen;

	/* generate seeds, store in ht */
	for (chainIndex = 0;  chainIndex < rainbowChainLen; ++chainIndex) {
		rainbowConfig->seeds[chainIndex] = getRandomULong();
	}

	/* dump to disk (will remain in mem until the ht is closed) */
	CHECK(DEHT_STATUS_FAIL != DEHT_writeUserBytes(ht));


	chainHashLen = getHashFunDigestLength(hashFunc);

	for (chainIndex = 1;  passwordEnumeratorCalculateNextPassword(passEnumerator);  ++chainIndex) {
		TRACE_FPRINTF(stderr, "TRACE: %s:%d (%s): working on chain %lu\r", __FILE__, __LINE__, __FUNCTION__, chainIndex);

		CHECK(buildChain(rainbowConfig, hashFunc,
				 (byte_t *) enumeratorPassword, strlen(enumeratorPassword),
			 	 passGenerator, (byte_t *) generatorPassword,
				 chainHash, sizeof(chainHash)));

		/* Insert resulting hash as the key and the initial password as the value */
		CHECK(DEHT_STATUS_FAIL != insert_uniquely_DEHT(ht, 
							       chainHash, chainHashLen,
							       (byte_t *) enumeratorPassword, strlen(enumeratorPassword)));
	}

	TRACE("chain building complete");



	ret = TRUE;
	goto LBL_CLEANUP;
	
	
LBL_ERROR:
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	if (NULL != ht) {
		lock_DEHT_files(ht);
		ht = NULL;
	}

	TRACE_FUNC_EXIT();

	return ret;
}

/*

bool_t openRainbowTable(RainbowTable_t * instance,
			const passwordGenerator_t * passGenerator,
			char * generatorPassword,

			BasicHashFunctionPtr hashFunc,

			ulong_t rainbowChainLen,

			const char * hashTableFilePrefix,
			bool_t enableFirstBlockCache,
			bool_t enableLastBlockCache)
{
	bool_t ret = FALSE;

	DEHT * ht = NULL;

	ulong_t chainIndex = 0;
	ulong_t inChainIndex = 0;

	RainbowTableConfig_t * rainbowConfig = NULL;
	ulong_t userBytesSize = 0;

	ulong_t hashLen = 0;
	byte_t currHash[MAX_DIGEST_LEN];

	/* "k" */
/*	ulong_t nextPasswordIndex = 0;


	TRACE_FUNC_ENTRY();

	CHECK(NULL != passGenerator);
	CHECK(NULL != enumeratorPassword);
	CHECK(NULL != generatorPassword);
	CHECK(NULL != hashFunc);
	CHECK(NULL != hashTableFilePrefix);

	ht = create_empty_DEHT(hashTableFilePrefix, 
			       DEHT_keyToTableIndexHasher, DEHT_keyToValidationKeyHasher64, 
			       getNameFromHashFun(hashFunc), 
			       nHashTableEntries, nPairsPerBlock, sizeof(ulong_t), sizeof(RainbowTableConfig_t) + sizeof(RainbowSeed_t) * rainbowChainLen);
	CHECK(NULL != ht);

	if (enableFirstBlockCache) {
		CHECK(DEHT_STATUS_FAIL != read_DEHT_pointers_table(ht));
	}
	if (enableLastBlockCache) {
		CHECK(DEHT_STATUS_FAIL != calc_DEHT_last_block_per_bucket(ht));
	}

	


	/* load user bytes buffer (even though it should be empty right now, this will allow us to use the table's buffer, which gets saved to disk automagically) */
/*	CHECK(readUserBytes(ht, &seeds, &userBytesSize));
	CHECK(NULL != seeds);
	CHECK(userBytesSize == rainbowChainLen * sizeof(RainbowSeed_t));
	/* generate seeds, store in ht */
/*	for (inChainIndex = 0;  inChainIndex < rainbowChainLen; ++inChainIndex) {
		seeds[inChainIndex] = getRandomULong();
	}


}

bool_t closeRainbowTable(RainbowTable_t * instance);
*/
