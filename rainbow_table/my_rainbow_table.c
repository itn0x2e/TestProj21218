#include "../common/constants.h"
#include "../common/misc.h"
#include "../common/rand_utils.h"
#include "../common/utils.h"
#include "../DEHT/hash_funcs.h"
#include "../password/password_enumerator.h"
#include "my_rainbow_table.h"

static DEHT * createEmptyRainbowTable(const char *prefix,
				      const char *dictName,
				      uint_t numEntriesInHashTable,
				      uint_t nPairsPerBlock,
				      uint_t nBytesPerKey,
				      uint_t rainbowChainLen);
static void closeRainbowTable(DEHT * rainbowTable);
static const LONG_INDEX_PROJ * createSeeds(DEHT * rainbowTable, ulong_t rainbowChainLen);
static void randomizeSeeds(LONG_INDEX_PROJ * seeds, ulong_t rainbowChainLen);
static bool_t fillRainbowTable(DEHT * deht,
			       passwordEnumerator_t * passwordEnumerator,
			       const passwordGenerator_t * passwordGenerator,
			       char * firstPass,
			       char * kthPass,
			       BasicHashFunctionPtr cryptHashPtr,
			       ulong_t chainLength,
			       const LONG_INDEX_PROJ * seeds);
static bool_t insertIntoDEHT(DEHT * deht, const byte_t * key, uint_t keyLen, const char * dataStr);

bool_t RT_generate(passwordEnumerator_t * passwordEnumerator,
		   const passwordGenerator_t * passwordGenerator,
		   char * enumeratorPassword,
		   char * generatorPassword,
		   BasicHashFunctionPtr hashFunc,
		   ulong_t rainbowChainLen,
		   const char * hashTableFilePrefix,
		   ulong_t nHashTableEntries,
		   ulong_t nPairsPerBlock,
		   bool_t b1, /*! TODO: tmp */
		   bool_t b2 /*! TODO: tmp */) {
	bool_t ret = FALSE;
	DEHT * rainbowTable = NULL;
	const LONG_INDEX_PROJ * seeds = NULL;
	
	/* Create an empty rainbow table */
	CHECK(rainbowTable = createEmptyRainbowTable(hashTableFilePrefix,
						     getNameFromHashFun(hashFunc),
						     nHashTableEntries,
						     nPairsPerBlock,
						     BYTES_PER_KEY,
						     (uint_t) rainbowChainLen));
	
	/* Create seeds and store them in the rainbow table */
	CHECK(seeds = createSeeds(rainbowTable, rainbowChainLen));
	
	/* Fill the rainbow table */
	ret = fillRainbowTable(rainbowTable,
			       passwordEnumerator,
			       passwordGenerator,
			       enumeratorPassword,
			       generatorPassword,
			       hashFunc,
			       rainbowChainLen,
			       seeds);
	       
LBL_ERROR:
	closeRainbowTable(rainbowTable);
	/*!TODO: should we delete the DEHT files upon failure? */
	/*! TODO: should I really avoid FREE(seeds)? */
	return ret;
}

RainbowTable_t * RT_open(const passwordGenerator_t * passGenerator,
			 char * generatorPassword,
			 const char * hashTableFilePrefix,
			 bool_t b1, /*! TODO: tmp */
			 bool_t b2 /*! TODO: tmp */) {
	/*!TODO: dummy*/
	return NULL;
}

void RT_close(RainbowTable_t * self) {
	/*!TODO: dummy*/
}

bool_t RT_query(RainbowTable_t * self, 
		byte_t * hash, ulong_t hashLen,
		char * resPassword, ulong_t resPasswordLen) {
	/*!TODO: dummy*/
	return FALSE;
}
		
static DEHT * createEmptyRainbowTable(const char *prefix,
				      const char *dictName,
				      uint_t numEntriesInHashTable,
				      uint_t nPairsPerBlock,
				      uint_t nBytesPerKey,
				      uint_t rainbowChainLen) {
	return create_empty_DEHT(prefix,
				 DEHT_keyToTableIndexHasher, DEHT_keyToValidationKeyHasher64,
				 dictName,
				 (int) numEntriesInHashTable,
				 (int) nPairsPerBlock,
				 (int) nBytesPerKey,
				 (int) (sizeof(ulong_t) + (rainbowChainLen * sizeof(LONG_INDEX_PROJ))));
}

static void closeRainbowTable(DEHT * rainbowTable) {
	if (NULL != rainbowTable) {
		lock_DEHT_files(rainbowTable);
	}
}

static const LONG_INDEX_PROJ * createSeeds(DEHT * rainbowTable, ulong_t rainbowChainLen) {
	byte_t * buf = NULL;
	LONG_INDEX_PROJ * seeds = NULL;
	ulong_t bufSize = 0;
	
	/* Read user bytes */
	CHECK(DEHT_STATUS_FAIL != DEHT_getUserBytes(rainbowTable, &buf, &bufSize))
	
	/* Write the rainbow chain length and seeds into those bytes */
	if ((sizeof(ulong_t) + (rainbowChainLen * sizeof(LONG_INDEX_PROJ))) != bufSize) {
		fprintf(stderr, "Error: DEHT file corruption");
		goto LBL_ERROR;
	}
	*((ulong_t *) buf) = rainbowChainLen;
	seeds = (LONG_INDEX_PROJ *) (buf + sizeof(ulong_t));
	randomizeSeeds(seeds, rainbowChainLen);
	
	/* Store the user bytes */
	CHECK(DEHT_STATUS_FAIL != DEHT_writeUserBytes(rainbowTable));
	
	return seeds;

LBL_ERROR:
	return NULL;
}

static void randomizeSeeds(LONG_INDEX_PROJ * seeds, ulong_t rainbowChainLen) {
	ulong_t i;
	for (i = 0; i < rainbowChainLen; ++i) {
		seeds[i] = getRandomLongIndexProj();
		printf("Seed #%08ld: %08lX\n", i, seeds[i]); /*! TODO: tmp */
	}
}

static bool_t fillRainbowTable(DEHT * deht,
			       passwordEnumerator_t * passwordEnumerator,
			       const passwordGenerator_t * passwordGenerator,
			       char * firstPass,
			       char * kthPass,
			       BasicHashFunctionPtr cryptHashPtr,
			       ulong_t chainLength,
			       const LONG_INDEX_PROJ * seeds) {
	const ulong_t numPossiblePasswords = passwordGeneratorGetSize(passwordGenerator);
	
	/* Iterate many times (about 10 times size of S) */
	while(passwordEnumeratorCalculateNextPassword(passwordEnumerator)) {
		/* Init curHash := cryptographic-hash(firstPass) */
		byte_t curHash[MAX_DIGEST_LEN];
		uint_t curHashLen = (uint_t) cryptHash(cryptHashPtr, firstPass, curHash);
		/* For j=1 to chain-length do */
		ulong_t j;
				
		/*		
		char outStr[1000];
		printf("firstPass == %s\n", firstPass);
		binary2hexa(curHash, curHashLen, outStr, 1000);
		printf("curHash == %s\n", outStr);
		*/
		
		
		for (j = 0; j < chainLength; ++j) {
			/* k = pseudo-random-function with seed seed[j] and input curHash; */
			LONG_INDEX_PROJ k = pseudo_random_function(curHash, curHashLen, seeds[j]); /*! TODO: yet to be implemented */
			
			/* curHash = cryptographic-hash(get_kth_password_64b(k,S)); */
			k %= numPossiblePasswords;
			passwordGeneratorCalculatePassword(passwordGenerator, k, kthPass);
			cryptHash(cryptHashPtr, kthPass, curHash);
			
			
			
			
			
			/*
			printf("kthPass == %s\n", kthPass);
			binary2hexa(curHash, curHashLen, outStr, 1000);
			printf("curHash == %s\n", outStr);
			*/
		/* end */
		}
		/*
		binary2hexa(curHash, curHashLen, outStr, 1000);
		printf("curHash == %s\n", outStr);
		*/
		
		/* insert into disk embedded hash table the following pair: key=curHash, data=firstPass */
		if (!insertIntoDEHT(deht, curHash, curHashLen, firstPass)) {
			printf("sfsdfsdfsdfsdafsadfasdfasdfssadfassfsadfasdf\n");
			return FALSE;
		}
	/* end */
	}
	return TRUE;
}

static bool_t insertIntoDEHT(DEHT * deht, const byte_t * key, uint_t keyLen, const char * dataStr) {
	char outStr[1000];
	binary2hexa(key, keyLen, outStr, 1000);
	printf("key == %s, data == %s\n", outStr, dataStr);
	return (DEHT_STATUS_FAIL != insert_uniquely_DEHT(deht, key, keyLen, (const unsigned char *) dataStr, strlen(dataStr)));
}