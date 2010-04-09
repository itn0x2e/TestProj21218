#include "my_rainbow_table.h"

static DEHT * createEmptyRainbowTable();
static LONG_INDEX_PROJ * createSeeds(DEHT * rainbowTable, ulong_t rainbowChainLen);     
static bool_t fillRainbowTable(passwordEnumerator_t * passwordEnumerator,
			       char * firstPass,
			       BasicHashFunctionPtr cryptHashPtr,
			       ulong_t chainLength,
			       LONG_INDEX_PROJ * seeds);
static void closeRainbowTable(DEHT * rainbowTable);

static bool_t insertIntoDEHT(const byte_t * key, uint_t keyLen, const char * dataStr);

bool_t RT_generate(passwordEnumerator_t * passwordEnumerator,
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
	LONG_INDEX_PROJ * seeds = NULL;
	
	/* Create an empty rainbow table */
	CHECK(rainbowTable = createEmptyRainbowTable());
	
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
	FREE(seeds);
	return ret;
}

RainbowTable_t * RT_open(const passwordGenerator_t * passGenerator,
			 char * generatorPassword,
			 const char * hashTableFilePrefix,
			 bool_t enableFirstBlockCache,
			 bool_t enableLastBlockCache);

void RT_close(RainbowTable_t * self);

bool_t RT_query(RainbowTable_t * self, byte_t * hash, ulong_t hashLen, char * resPassword, ulong_t resPasswordLen);

static DEHT * createEmptyRainbowTable(const char *prefix,
				      const char *dictName,
				      int numEntriesInHashTable,
				      int nPairsPerBlock,
				      int nBytesPerKey,
				      int nUserBytes) {
	return create_empty_DEHT(const char *prefix,
				 hashKeyIntoTableFunctionPtr hashfun,
				 hashKeyforEfficientComparisonFunctionPtr validfun,
				 const char *dictName,
				 int numEntriesInHashTable,
				 int nPairsPerBlock,
				 int nBytesPerKey,
				 int nUserBytes
	/*!TODO: dummy*/
}

static bool_t fillRainbowTable(passwordEnumerator_t * passwordEnumerator,
			       const passwordGenerator_t * passwordGenerator,
			       char * firstPass,
			       char * kthPass,
			       BasicHashFunctionPtr cryptHashPtr,
			       ulong_t chainLength,
			       LONG_INDEX_PROJ * seeds) {
	/* Iterate many times (about 10 times size of S) */
	while(passwordEnumeratorCalculateNextPassword()) {
		/* Init curHash := cryptographic-hash(firstPass) */
		byte_t curHash[MAX_DIGEST_LEN];
		uint_t curHashLen = (uint_t) cryptHash(cryptHashPtr, firstPass, curHash);
		/* For j=1 to chain-length do */
		ulong_t j;
		for (j = 0; j < chainLength; ++j) {
			/* k = pseudo-random-function with seed seed[j] and input curHash; */
			LONG_INDEX_PROJ k = pseudo_random_function(curHash, curHashLen, seed[j]);
			passwordGeneratorCalculatePassword(passwordGenerator, k, kthPass);
			/* curHash = cryptographic-hash(get_kth_password_64b(k,S)); */
			cryptHash(cryptHashPtr, kthPass, curHash);
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

static void closeRainbowTable(DEHT * rainbowTable) {
	if (NULL != rainbowTable) {
		lock_DEHT_files(rainbowTable);
	}
}