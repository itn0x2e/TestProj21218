#include "my_rainbow_table.h"

static DEHT * createEmptyRainbowTable();
static void closeRainbowTable(DEHT * rainbowTable);
static const LONG_INDEX_PROJ * createSeeds(DEHT * rainbowTable, ulong_t rainbowChainLen);
static void randomizeSeeds(LONG_INDEX_PROJ * seeds, ulong_t rainbowChainLen)
static bool_t fillRainbowTable(passwordEnumerator_t * passwordEnumerator,
			       char * firstPass,
			       BasicHashFunctionPtr cryptHashPtr,
			       ulong_t chainLength,
			       LONG_INDEX_PROJ * seeds);
static bool_t insertIntoDEHT(DEHT * deht, const byte_t * key, uint_t keyLen, const char * dataStr);

bool_t rainbowTableGenerate(passwordEnumerator_t * passwordEnumerator,
			    const passwordGenerator_t * passwordGenerator,
			    char * enumeratorPassword,
			    char * generatorPassword,
			    BasicHashFunctionPtr hashFunc,
			    ulong_t rainbowChainLen,
			    const char * hashTableFilePrefix,
			    ulong_t nHashTableEntries,
			    ulong_t nPairsPerBlock) {
	bool_t ret = FALSE;
	DEHT * rainbowTable = NULL;
	const LONG_INDEX_PROJ * seeds = NULL;
	
	/* Create an empty rainbow table */
	CHECK(rainbowTable = createEmptyRainbowTable(hashTableFilePrefix,
						     getNameFromHashFun(hashFunc),
						     nHashTableEntries,
						     nPairsPerBlock,
						     10 /*! TODO: nBytesPerKey */
						     10000 /*! TODO: nUserBytes*/));
	
	/* Create seeds and store them in the rainbow table */
	CHECK(seeds = generateSeeds(rainbowTable, rainbowChainLen));
	
	/* Fill the rainbow table */
	ret = fillRainbowTable(passwordEnumerator,
			       passwordGenerator,
			       enumeratorPassword,
			       generatorPassword,
			       hashFunc,
			       rainbowChainLen,
			       seeds);
			       
LBL_ERROR:
	closeRainbowTable(DEHT * rainbowTable);
	/*!TODO: should we delete the DEHT files upon failure? */
	FREE(seeds);
	return ret;
}

static DEHT * createEmptyRainbowTable(const char *prefix,
				      const char *dictName,
				      uint_t numEntriesInHashTable,
				      uint_t nPairsPerBlock,
				      uint_t nBytesPerKey,
				      uint_t nUserBytes) {
	return create_empty_DEHT(const char *prefix,
				 NULL, /*!TODO: hashKeyIntoTableFunctionPtr hashfun,*/
				 NULL, /*!TODO: hashKeyforEfficientComparisonFunctionPtr validfun,*/
				 NULL, /*!TODO: const char *dictName, */
				 (int) numEntriesInHashTable,
				 (int) nPairsPerBlock,
				 (int) nBytesPerKey,
				 (int) nUserBytes)
}

static void closeRainbowTable(DEHT * rainbowTable) {
	if (NULL != rainbowTable) {
		lock_DEHT_files(rainbowTable);
	}
}

static LONG_INDEX_PROJ * createSeeds(DEHT * rainbowTable, ulong_t rainbowChainLen) {
	LONG_INDEX_PROJ * seeds = randomizeSeeds(rainbowChainLen);
	if (NULL == seeds) {
		return NULL;
	}
	if (!storeSeeds(rainbowTable, seeds, rainbowChainLen)) { /*! TODO: is it OK even when rainbowChainLen == 0 ? */
		FREE(seeds);
	}
		
	return seeds;
}

static const LONG_INDEX_PROJ * createSeeds(DEHT * rainbowTable, ulong_t rainbowChainLen) {
	const ulong_t numBytesToWrite = rainbowChainLen * sizeof(*seeds);
	void * buf = NULL;
	ulong_t bufSize = 0;
	
	/* Read user bytes */
	CHECK(DEHT_STATUS_FAIL != DEHT_readUserBytes(rainbowTable, &buf, &bufSize))
	
	/* Write the seeds into those bytes */
	CHECK(bufSize >= (rainbowChainLen * sizeof(LONG_INDEX_PROJ));
	randomizeSeeds((LONG_INDEX_PROJ *) buf, rainbowChainLen);
	
	/* Store the user bytes */
	CHECK(DEHT_STATUS_FAIL != DEHT_writeUserBytes(rainbowTable));
	
	return buf;

LBL_ERROR:
	return NULL;
}

static void randomizeSeeds(LONG_INDEX_PROJ * seeds, ulong_t rainbowChainLen) {
	for (i = 0; i < rainbowChainLen; ++i) {
		seeds[i] = getRandomUlong(); /*! TODO: consider using pseudo_random_function */
		printf(seeds[i]); /*! TODO: tmp */
	}
}

static bool_t fillRainbowTable(passwordEnumerator_t * passwordEnumerator,
			       const passwordGenerator_t * passwordGenerator,
			       char * firstPass,
			       char * kthPass,
			       BasicHashFunctionPtr cryptHashPtr,
			       ulong_t chainLength,
			       const LONG_INDEX_PROJ * seeds) {
	/* Iterate many times (about 10 times size of S) */
	while(passwordEnumeratorCalculateNextPassword()) {
		/* Init curHash := cryptographic-hash(firstPass) */
		byte_t curHash[MAX_DIGEST_LEN];
		uint_t curHashLen = (uint_t) cryptHash(cryptHashPtr, firstPass, curHash);
		/* For j=1 to chain-length do */
		ulong_t j;
		for (j = 0; j < chainLength; ++j) {
			/* k = pseudo-random-function with seed seed[j] and input curHash; */
			LONG_INDEX_PROJ k = pseudo_random_function(curHash, curHashLen, seed[j]); /*! TODO: yet to be implemented */
			
			/* curHash = cryptographic-hash(get_kth_password_64b(k,S)); */
			passwordGeneratorCalculatePassword(passwordGenerator, k, kthPass);
			cryptHash(cryptHashPtr, get_password_64b(kthPass), 8);
		/* end */
		}
		/* insert into disk embedded hash table the following pair: key=curHash, data=firstPass */
		if (!insertIntoDEHT(curHash, curHashLen, firstPass)) {
			return FALSE;
		}
	/* end */
	}
	return TRUE;
}

static bool_t insertIntoDEHT(DEHT * deht, const byte_t * key, uint_t keyLen, const char * dataStr) {
	return (DEHT_STATUS_FAIL != insert_uniquely_DEHT(deht, key, keyLen, (const unsigned char *) dataStr, strlen(dataStr)));
}