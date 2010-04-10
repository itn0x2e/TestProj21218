#include "../common/constants.h"
#include "../common/misc.h"
#include "../common/rand_utils.h"
#include "../common/utils.h"
#include "../DEHT/hash_funcs.h"
#include "../password/password_enumerator.h"
#include "rainbow_table.h"

static DEHT * createEmptyRainbowTable(const char *prefix,
	      const char *dictName,
	      uint_t numEntriesInHashTable,
	      uint_t nPairsPerBlock,
	      uint_t nBytesPerKey,
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

static bool_t getConfigSize(ulong_t chainLength);

static bool_t insertIntoDEHT(DEHT * deht, const byte_t * key, uint_t keyLen, const char * dataStr);

static bool_t queryPasswordFromDEHT(DEHT * deht,
				    const byte_t * key,
				    uint_t keyLen,
				    char * dataStr,
				    uint_t dataStrMaxLen,
				    bool_t * found);
   

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
	const RainbowTableConfig_t * config = NULL;
	
	/* Create an empty rainbow table */
	CHECK(rainbowTable = createEmptyRainbowTable(hashTableFilePrefix,
						     getNameFromHashFun(hashFunc),
						     nHashTableEntries,
						     nPairsPerBlock,
						     BYTES_PER_KEY,
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
	/*!TODO: should we delete the DEHT files upon failure? */
	/*! TODO: should I really avoid FREE(seeds)? */
	return ret;
}

RainbowTable_t * RT_open(const passwordGenerator_t * passGenerator,
			 char * password,
			 ulong_t passwordLength,
			 const char * hashTableFilePrefix,
			 bool_t enableFirstBlockCache,
			 bool_t enableLastBlockCache) {
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

	/* we will be generating a lot of query operations - better use DEHT's first block cache */
	/* (Note, however, that errors here are treated as non-terminal, to increase robustness) */
	(void) read_DEHT_pointers_table(self->hashTable);


	/* find the hash type */
	self->hashFunc = getHashFunFromName(self->hashTable->header.sDictionaryName);
	CHECK(NULL != self->hashFunc);

	/* load user bytes buffer */
	CHECK(readConfig(self->hashTable, &(self->config)));

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
				      uint_t nPairsPerBlock,
				      uint_t nBytesPerKey,
				      uint_t configLength) {
	return create_empty_DEHT(prefix,
				 DEHT_keyToTableIndexHasher, DEHT_keyToValidationKeyHasher64,
				 dictName,
				 (int) numEntriesInHashTable,
				 (int) nPairsPerBlock,
				 (int) nBytesPerKey,
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
			       const RainbowSeed_t * seeds) {
	const ulong_t numPossiblePasswords = passwordGeneratorGetSize(passwordGenerator);
	
	/* Iterate many times (about 10 times size of S) */
	while(passwordEnumeratorCalculateNextPassword(passwordEnumerator)) {
		/* Init curHash := cryptographic-hash(firstPass) */
		byte_t curHash[MAX_DIGEST_LEN];
		uint_t curHashLen = (uint_t) cryptHash(cryptHashPtr, firstPass, curHash);
		/* For j=1 to chain-length do */
		ulong_t j;
				

		char outStr[1000];
		printf("firstPass == %s\n", firstPass);
		binary2hexa(curHash, curHashLen, outStr, 1000);
		printf("curHash == %s\n", outStr);
		
		

		for (j = 0; j < chainLength; ++j) {
			/* k = pseudo-random-function with seed seed[j] and input curHash; */
			RainbowSeed_t k = pseudo_random_function(curHash, curHashLen, seeds[j]); /*! TODO: yet to be implemented */
			
			/* curHash = cryptographic-hash(get_kth_password_64b(k,S)); */
			k %= numPossiblePasswords;
			passwordGeneratorCalculatePassword(passwordGenerator, k, kthPass);
			cryptHash(cryptHashPtr, kthPass, curHash);
			
			
			
			
			

			printf("kthPass == %s\n", kthPass);
			binary2hexa(curHash, curHashLen, outStr, 1000);
			printf("curHash == %s\n", outStr);

		/* end */
		}
		/*
		binary2hexa(curHash, curHashLen, outStr, 1000);
		printf("curHash == %s\n", outStr);
		*/
		
		/* insert into disk embedded hash table the following pair: key=curHash, data=firstPass */
		printf("finished iteration\n");
		if (!insertIntoDEHT(deht, curHash, curHashLen, firstPass)) {
			printf("sfsdfsdfsdfsdafsadfasdfasdfssadfassfsadfasdf\n");
			return FALSE;
		}
	/* end */
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
	
	ulong_t numPossiblePasswords = passwordGeneratorGetSize(passwordGenerator);

	char * tryThisPassword = malloc((passwordLen + 1) * sizeof(char)); /*! TODO: can't i just use 'password' only? */
	if (NULL == tryThisPassword) {
		PERROR();
		return FALSE;
	}
	
	/*! Initially, the respose is that a password matching this hash value cannot be found */
	*found = FALSE;

	/*! If the length of the target hash value is wrong, is is useless to look for it */
	if (targetLen != getHashFunDigestLength(cryptHashPtr)) {
		/* Return immidiately, informing that the query was performed correctly,
		* but a password matching this hash value cannot be found */
		return TRUE;
	}
	
	/* For j=1 to chain-length do */
	/*! I added (j == 0) too */
	for (j = 0; j <= chainLength; ++j) {
		/* Gamble that our password is location "j" in some chain as follow:*/
		byte_t curHash[MAX_DIGEST_LEN];
		const uint_t curHashLen = targetLen;
		bool_t foundInJ = FALSE;

		/* go down the chain j steps (and get curHash j steps downstream).*/
		memcpy(curHash, target, targetLen); /*! TODO: is that right? */
		goDownChain(curHash, curHashLen, passwordGenerator, numPossiblePasswords, password, cryptHashPtr, seeds, j);
		
		/* query in disc-embedded hash table with key: curHash.
		 * Get data (a password) call it: tryThisPassword */
		CHECK(queryPasswordFromDEHT(deht, curHash, curHashLen, tryThisPassword, passwordLen, &foundInJ));

               /* If tryThisPassword is NULL then continue loop (obviously wrong j)
                * Else, assume tryThisPassword is beginning of correct chain so */
	       if (foundInJ) {
                        /* Init curPass:= tryThisPassword */
			strcpy(password, tryThisPassword);
			
                        /* Go chain-length -j steps down (i.e. to the correct location). */
			goDownChain(curHash, curHashLen, passwordGenerator, numPossiblePasswords, password, cryptHashPtr, seeds, chainLength - j);
			
                        /* Check whether cryptographic-hash(curPass)==target */
			/*!TODO: the following seems to me unnecesarry and wrong: cryptHash(cryptHashPtr, curPass, curHash);*/
			if (0 == memcmp(curHash, target, targetLen)) {
				/* If so, return curPass */
				*found = TRUE;
				break;
			} /* Else, seek for a different j (i.e. false alarm). */ 
	       } /* End (conditions) */
	} /* End (main loop on j) */
	
	/* The query was performed correctly */
	return TRUE;
	
LBL_ERROR:
	return FALSE;
}

static bool_t getConfigSize(ulong_t chainLength) {
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
printf("(*config)->chainLength) == %d\n", (*config)->chainLength);
	fprintf(stderr, "Error: DEHT file corruption\n");
	return FALSE;
}

static bool_t insertIntoDEHT(DEHT * deht, const byte_t * key, uint_t keyLen, const char * dataStr) {
	char outStr[1000];
	binary2hexa(key, keyLen, outStr, 1000);
	printf("INSERT\tkey == %s, data == %s\n", outStr, dataStr);
	/*! TODO: document that we do put the terminating null */
	return (DEHT_STATUS_FAIL != insert_uniquely_DEHT(deht, key, keyLen, (const unsigned char *) dataStr + 1, strlen(dataStr) + 1));
}

static bool_t queryPasswordFromDEHT(DEHT * deht,
				    const byte_t * key,
				    uint_t keyLen,
				    char * dataStr,
				    uint_t dataStrMaxLen,
				    bool_t * found) {
	
	int queryRet;
	
	char outStr[1000];
	binary2hexa(key, keyLen, outStr, 1000);
	printf("QUERY\tkey == %s\n", outStr);
	
	
	
	/*! TODO: document that we do read the terminating null */
	queryRet = query_DEHT(deht, key, keyLen, (byte_t *) dataStr, dataStrMaxLen + 1);
	
	switch(queryRet) {
		case DEHT_STATUS_NOT_NEEDED:
			*found = FALSE;
			return TRUE;
		case DEHT_STATUS_FAIL:
			return FALSE;
	}
	
	if ('\0' != dataStr[queryRet]) {
		/* The data read is not a string */
		return FALSE;
	}

	*found = TRUE;
	
	

	printf("FOUND\tkey == %s, data == %s\n", outStr, dataStr);
	
	
	return TRUE;
}

void goDownChain(byte_t * curHash,
			uint_t curHashLen,
			const passwordGenerator_t * passwordGenerator,
			ulong_t numPossiblePasswords,
			char * pass,
			BasicHashFunctionPtr cryptHashPtr,
			const RainbowSeed_t * seeds,
			ulong_t iterations) {
	ulong_t j;


	char outStr[1000];
	printf("password == %s\n", pass);
	binary2hexa(curHash, curHashLen, outStr, 1000);
	printf("curHash == %s\n", outStr);



	for (j = 0; j < iterations; ++j) {
		/* k = pseudo-random-function with seed seed[j] and input curHash; */
		RainbowSeed_t k = pseudo_random_function(curHash, curHashLen, seeds[j]); /*! TODO: yet to be implemented */

		/* curHash = cryptographic-hash(get_kth_password_64b(k,S)); */
		k %= numPossiblePasswords;
		passwordGeneratorCalculatePassword(passwordGenerator, k, pass);
		cryptHash(cryptHashPtr, pass, curHash);






		printf("password == %s\n", pass);
		binary2hexa(curHash, curHashLen, outStr, 1000);
		printf("curHash == %s\n", outStr);

	/* end */
	}
}
