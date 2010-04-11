#include "../common/constants.h"
#include "../common/misc.h"
#include "../common/rand_utils.h"
#include "../common/utils.h"
#include "../DEHT/hash_funcs.h"
#include "../password/password_enumerator.h"
#include "rainbow_table.h"


/**
* Create an empty Rainbow Table according to the specified parameters. This is only a creation interface
* for queries, see RT_open() & RT_query().
* Function desc: This function will generate a new Rainbow Table as per the user's request.
*
* @param prefix - file prefix for DEHT hash table files. files are generated
*			       for hashTableFilePrefix.key and hashTableFilePrefix.data
* @param dictName - filename for dictionary
* @param numEntriesInHashTable - number of buckets in the hash table. This may require some 
*			     tuning to get decent performance.
*			  performance.
* @param nPairsPerBlock - number of key<->data pairs to store in each disk block in the 
*			  hash table. This may require some tuning to get decent performance.
* @param passwordEnumerator - a password enumerator over the password space. Expected to be a random
*			      password enumerator over the password range, of about 10 * the actual
*			      range (to increase likelihood of a hit for queries)
* @param passwordGenerator - a password generator (random access to the entire password range)
* @param enumeratorPassword - the buffer associated with the password enumerator
* @param generatorPassword - the buffer associated with the password generator
* @param hashFunc - valid pointer the one of the hash function (SHA-1 / MD-5)
* @param rainbowChainLen - length of the rainbow-chains
* @param prefix - file prefix for DEHT hash table files. files are generated
*			       for hashTableFilePrefix.key and hashTableFilePrefix.data
*
* @ret TRUE on success, FALSE otherwise.
*
*/
static DEHT * createEmptyRainbowTable(const char *prefix,
	      const char *dictName,
	      uint_t numEntriesInHashTable,
	      uint_t nPairsPerBlock,
	      uint_t configLength);
				      
static void closeRainbowTable(DEHT * rainbowTable);

static const RainbowTableConfig_t * createConfig(DEHT * rainbowTable, ulong_t rainbowChainLen);

static void randomizeSeeds(RainbowSeed_t * seeds, ulong_t rainbowChainLen);

static bool_t fillRainbowTable(DEHT * deht,
			       passwordEnumerator_t * passwordEnumerator,
			       const passwordGenerator_t * passwordGenerator,
			       char * firstPass,
			       char * kthPass,
			       BasicHashFunctionPtr cryptHashPtr,
			       ulong_t chainLength,
			       const RainbowSeed_t * seeds);
			       
static bool_t queryRainbowTable(DEHT * deht,
		const passwordGenerator_t * passwordGenerator,
		char * password,
		ulong_t passwordLen,
		BasicHashFunctionPtr cryptHashPtr,
		ulong_t chainLength,
		const RainbowSeed_t * seeds,
		const byte_t * target,
		uint_t targetLen,
		bool_t * found);
				
static bool_t readConfig(DEHT * deht, RainbowTableConfig_t ** config);


static bool_t insertIntoDEHT(DEHT * deht, const byte_t * key, uint_t keyLen, const char * dataStr);

static bool_t queryPasswordFromDEHT(DEHT * deht,
				    const byte_t * key,
				    uint_t keyLen,
				    char * dataStr,
				    uint_t dataStrMaxLen,
				    bool_t * found);
   
static void advanceInChain(byte_t * hash,
			   uint_t hashLen,
			   const passwordGenerator_t * passwordGenerator,
			   ulong_t numPossiblePasswords,
			   char * password,
			   BasicHashFunctionPtr cryptHashPtr,
			   const RainbowSeed_t * seeds,
			   uint_t steps,
			   uint_t beginningIndex);

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
	const RainbowTableConfig_t * config = NULL;

	/* Create an empty rainbow table */
	CHECK(rainbowTable = createEmptyRainbowTable(hashTableFilePrefix,
						     getNameFromHashFun(hashFunc),
						     nHashTableEntries,
						     nPairsPerBlock,
						     getConfigSize(rainbowChainLen)));

	/* Create rainbow table configuration in the rainbow table */
	config = createConfig(rainbowTable, rainbowChainLen);
	CHECK(NULL != config);

	/* Fill the rainbow table */
	ret = fillRainbowTable(rainbowTable,
			       passwordEnumerator,
			       passwordGenerator,
			       enumeratorPassword,
			       generatorPassword,
			       hashFunc,
			       rainbowChainLen,
			       config->seeds);

LBL_ERROR:
	closeRainbowTable(rainbowTable);

	/* We failed to generate the table, so we should remove the file */
	(void) DEHT_removeFiles(hashTableFilePrefix);

	return ret;
}

RainbowTable_t * RT_open(const passwordGenerator_t * passGenerator,
			 char * password,
			 ulong_t passwordLength,
			 const char * hashTableFilePrefix,
			 bool_t enableFirstBlockCache) {
	RainbowTable_t * ret = NULL;
	RainbowTable_t * self = NULL;
	
	TRACE_FUNC_ENTRY();

	CHECK(NULL != hashTableFilePrefix);

	self = (RainbowTable_t *) malloc(sizeof(RainbowTable_t));
	CHECK(NULL != self);

	/* init */
	memset(self, 0, sizeof(RainbowTable_t));

	self->passGenerator = passGenerator;
	self->password = password;
	self->passwordLength = passwordLength;

	self->hashTable = load_DEHT_from_files(hashTableFilePrefix,
			       DEHT_keyToTableIndexHasher, DEHT_keyToValidationKeyHasher64);
	CHECK(NULL != self->hashTable);

	if (enableFirstBlockCache) {
		/* Apparently, we will be generating a lot of query operations - better use DEHT's first block cache */
		/* (Note, however, that errors here are treated as non-terminal, to increase robustness) */
		(void) read_DEHT_pointers_table(self->hashTable);
	}

	/* find the hash type */
	self->hashFunc = getHashFunFromName(self->hashTable->header.sDictionaryName);
	CHECK(NULL != self->hashFunc);

	/* load user bytes buffer */
	CHECK(readConfig(self->hashTable, &(self->config)));
	/*{int  i; for(i=0;i< self->config->chainLength; ++i) printf("seed[%u] == %08X\n", i, self->config->seeds[i]);}*/
	CHECK(NULL != password);

	/* if chain length is 0, we will not require a password generator (otherwise, one is neccessary) */
	if (0 != self->config->chainLength) {
		CHECK(NULL != passGenerator);
	}

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

void RT_close(RainbowTable_t * self) {
	TRACE_FUNC_ENTRY();
	CHECK(NULL != self);
	
	if (NULL != self->hashTable) {
		lock_DEHT_files(self->hashTable);
		self->hashTable = NULL;
	}
	/* No need to free config - is handled by hashTable */
	self->config = NULL;

	/* No need to free passGenerator - handled by whoever supplied it */
	self->passGenerator = NULL;

	FREE(self);
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return;
}


bool_t RT_query(RainbowTable_t * self, const byte_t * hash, ulong_t hashLen, bool_t * found) {
	if (!queryRainbowTable(self->hashTable,
				self->passGenerator,
				self->password,
				self->passwordLength,
				self->hashFunc,
				self->config->chainLength,
				self->config->seeds,
				hash,
				hashLen,
				found)) {
		return FALSE;
	}

	return TRUE;
}
		
static DEHT * createEmptyRainbowTable(const char *prefix,
				      const char *dictName,
				      uint_t numEntriesInHashTable,
				      uint_t nPairsPerBlock, uint_t configLength) {
	return create_empty_DEHT(prefix,
				 DEHT_keyToTableIndexHasher, DEHT_keyToValidationKeyHasher64,
				 dictName,
				 (int) numEntriesInHashTable,
				 (int) nPairsPerBlock,
				 BYTES_PER_KEY,
				 (int) configLength);
}

static void closeRainbowTable(DEHT * rainbowTable) {
	if (NULL != rainbowTable) {
		lock_DEHT_files(rainbowTable);
	}
}

static const RainbowTableConfig_t * createConfig(DEHT * rainbowTable, ulong_t rainbowChainLen) {
	RainbowTableConfig_t * config = NULL;
	ulong_t size = 0;

	/* Read config */
	CHECK(DEHT_STATUS_FAIL != DEHT_getUserBytes(rainbowTable, ((byte_t **) &config), &size));
	if (getConfigSize(rainbowChainLen) != size) {
		fprintf(stderr, "Error: DEHT file corruption");
		goto LBL_ERROR;
	}
	
	/* Write the rainbow chain length and seeds into the config */
	config->chainLength = rainbowChainLen;
	randomizeSeeds(config->seeds, config->chainLength);
	
	/* Store the config */
	CHECK(DEHT_STATUS_FAIL != DEHT_writeUserBytes(rainbowTable));
	
	return config;

LBL_ERROR:
	return NULL;
}

static void randomizeSeeds(RainbowSeed_t * seeds, ulong_t rainbowChainLen) {
	ulong_t i;
	for (i = 0; i < rainbowChainLen; ++i) {
		seeds[i] = getRandomLongIndexProj();
		/*printf("Seed #%08ld: %08lX\n", i, seeds[i]);*/
	}
}

static bool_t fillRainbowTable(DEHT * deht,
			       passwordEnumerator_t * passwordEnumerator,
			       const passwordGenerator_t * passwordGenerator,
			       char * firstPass,
			       char * kthPass,
			       BasicHashFunctionPtr cryptHashPtr,
			       ulong_t chainLength,
			       const RainbowSeed_t * seeds) {
	while(passwordEnumeratorCalculateNextPassword(passwordEnumerator)) {
		/* Init curHash := cryptographic-hash(firstPass) */
		byte_t curHash[MAX_DIGEST_LEN];
		uint_t curHashLen = (uint_t) cryptHash(cryptHashPtr, firstPass, curHash);
		
		advanceInChain(curHash,
			       curHashLen,
			       passwordGenerator,
			       passwordGeneratorGetSize(passwordGenerator),
			       kthPass,
			       cryptHashPtr,
			       seeds,
			       chainLength,
			       0);

		if (!insertIntoDEHT(deht, curHash, curHashLen, firstPass)) {
			return FALSE;
		}
	}
	return TRUE;
}

static bool_t queryRainbowTable(DEHT * deht,
				const passwordGenerator_t * passwordGenerator,
				char * password,
				ulong_t passwordLen,
				BasicHashFunctionPtr cryptHashPtr,
				ulong_t chainLength,
				const RainbowSeed_t * seeds,
				const byte_t * target,
				uint_t targetLen,
				bool_t * found) {
	ulong_t j;

	/* Initially, the respose is that a password matching this hash value cannot be found */
	*found = FALSE;

	/* If the length of the target hash value is wrong, iא is useless to look for it */
	if (targetLen != getHashFunDigestLength(cryptHashPtr)) {
		/* Return immidiately, informing that the query was performed correctly,
		* but a password matching this hash value cannot be found */
		return TRUE;
	}
	
	for (j = 0; j <= chainLength; ++j) {
		/* Gamble that our password is location "j" in some chain as follow:*/
		byte_t curHash[MAX_DIGEST_LEN];
		const uint_t curHashLen = targetLen;
		bool_t foundInJ = FALSE;

		/* go down the chain j steps (and get curHash j steps downstream).*/
		memcpy(curHash, target, targetLen);
		advanceInChain(curHash,
			       curHashLen,
			       passwordGenerator,
			       passwordGeneratorGetSize(passwordGenerator),
			       password,
			       cryptHashPtr,
			       seeds,
			       j,
			       chainLength - j);

		/* query in disc-embedded hash table with key: curHash. */
		CHECK(queryPasswordFromDEHT(deht, curHash, curHashLen, password, passwordLen, &foundInJ));

		/* If not found then continue loop (obviously wrong j)
		 * Else, assume that this password is beginning of correct chain so */
		if (foundInJ) {
			/* Go chain-length -j steps down (i.e. to the correct location). */
			cryptHash(cryptHashPtr, password, curHash);
			advanceInChain(curHash,
					curHashLen,
					passwordGenerator,
					passwordGeneratorGetSize(passwordGenerator),
					password,
					cryptHashPtr,
					seeds,
					chainLength - j,
					0);

                        /* Check whether the hash of the current password is the target */
			cryptHash(cryptHashPtr, password, curHash);
			if (0 == memcmp(curHash, target, targetLen)) {
				/* If so, return the current password */
				*found = TRUE;
				break;
			}
	       }
	}
	
	/* The query was performed correctly */
	return TRUE;
	
LBL_ERROR:
	return FALSE;
}

ulong_t getConfigSize(ulong_t chainLength) {
	ulong_t size = sizeof(RainbowTableConfig_t);
	if (chainLength > 0) {
		size += (chainLength - 1) * sizeof(RainbowSeed_t);
	}
	return size;
}

static bool_t readConfig(DEHT * deht, RainbowTableConfig_t ** config) {
	ulong_t size;
	CHECK(DEHT_STATUS_FAIL != DEHT_getUserBytes(deht, (byte_t **) config, &size));
	CHECK(getConfigSize((*config)->chainLength) == size);

	return TRUE;

LBL_ERROR:
	fprintf(stderr, "Error: DEHT file corruption\n");
	return FALSE;
}

static bool_t insertIntoDEHT(DEHT * deht, const byte_t * key, uint_t keyLen, const char * dataStr) {
	return (DEHT_STATUS_FAIL != insert_uniquely_DEHT(deht,
							 key,
							 keyLen,
							 (const unsigned char *) dataStr,
							 strlen(dataStr) + 1));
}

static bool_t queryPasswordFromDEHT(DEHT * deht,
				    const byte_t * key,
				    uint_t keyLen,
				    char * dataStr,
				    uint_t dataStrMaxLen,
				    bool_t * found) {
	
	int queryRet = query_DEHT(deht, key, keyLen, (byte_t *) dataStr, dataStrMaxLen + 1);

	switch(queryRet) {
		case DEHT_STATUS_NOT_NEEDED:
			*found = FALSE;
			return TRUE;
		case DEHT_STATUS_FAIL:
			return FALSE;
	}

	if ('\0' != dataStr[queryRet - 1]) {
		/* The data read is not a string */
		fprintf(stderr, "Error: DEHT file corruption");
		return FALSE;
	}

	*found = TRUE;
	
	return TRUE;
}

static void advanceInChain(byte_t * hash,
			   uint_t hashLen,
			   const passwordGenerator_t * passwordGenerator,
			   ulong_t numPossiblePasswords,
			   char * password,
			   BasicHashFunctionPtr cryptHashPtr,
			   const RainbowSeed_t * seeds,
			   uint_t steps,
			   uint_t beginningIndex) {
	uint_t i;
	for (i=0; i < steps;++i) {
		/* k = pseudo-random-function with seed seed[j] and input curHash; */
		RainbowSeed_t k = pseudo_random_function(hash, hashLen, seeds[beginningIndex + i]);

		/* curHash = cryptographic-hash(get_kth_password_64b(k,S)); */
		passwordGeneratorCalculatePassword(passwordGenerator, k % numPossiblePasswords, password);
		cryptHash(cryptHashPtr, password, hash);
	}
}
