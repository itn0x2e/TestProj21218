
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







bool_t buildChain(bool_t crackingMode,

		         RainbowSeed_t * seeds, ulong_t chainLength, 
			 BasicHashFunctionPtr hashFunc,

			 const passwordGenerator_t * passGenerator, byte_t * generatorPassword,

			 byte_t * firstPassword, ulong_t firstPasswordLen,
			 byte_t * hashBuf, ulong_t hashBufLen,

			 byte_t * passwordOut, ulong_t passwordOutLen,

			 FILE * outputFD)
{
	bool_t ret = FALSE;

	ulong_t inChainIndex = 0;

	byte_t currHash[MAX_DIGEST_LEN];
	ulong_t hashLen = 0;

	char currHashStr[MAX_DIGEST_LEN * 2 + 1];

	/* "k" */
	ulong_t nextPasswordIndex = 0;


	TRACE_FUNC_ENTRY();

	CHECK(NULL != seeds);
	CHECK(NULL != hashFunc);

	CHECK(NULL != passGenerator);
	CHECK(NULL != generatorPassword);

	CHECK(NULL != hashBuf);

	if (crackingMode) {
		/* We were asked to crack a password - so we must start with the supplied hash */
		memcpy(currHash, hashBuf, MIN(sizeof(currHash), hashBufLen));
		hashLen = hashBufLen;

		if (NULL != outputFD) {
			binary2hexa(currHash, hashLen, currHashStr, sizeof(currHashStr));
			fprintf(outputFD, "%s\t", currHashStr);
		}
	}
	else {
		/* chain creation mode - start with the first password */
		CHECK(NULL != firstPassword);

		if (NULL != outputFD) {
			fprintf(outputFD, "%s\t", firstPassword);
		}

		hashLen = hashFunc(firstPassword, firstPasswordLen, currHash);
		CHECK(0 != hashLen);
		CHECK(hashLen <= hashBufLen);

		if (NULL != outputFD) {
			binary2hexa(currHash, hashLen, currHashStr, sizeof(currHashStr));
			fprintf(outputFD, "%s\t", currHashStr);
		}

		if (NULL != passwordOut) {
			SAFE_STRNCPY((char *) passwordOut, (char *) firstPassword, MIN(firstPasswordLen, passwordOutLen));
		}
	}

	/* perform the steps detailed in the project specification to compute the chain for this start point */
	for (inChainIndex = 0; inChainIndex < chainLength; ++inChainIndex) {

		TRACE_FPRINTF((stderr, "pass: %s, hash: %lu\n", generatorPassword, *((ulong_t *) currHash)));

		/* PRNG using the seed corresponding to the current inChainIndex */
		CHECK(0 != miniHash((byte_t *) &nextPasswordIndex, sizeof(nextPasswordIndex),
				    (byte_t *) (seeds + inChainIndex), sizeof(seeds[0]), 
				    currHash, hashLen));

		/* wrap-around the password space size */
		nextPasswordIndex = nextPasswordIndex % passwordGeneratorGetSize(passGenerator);

		/* generate k-randomized password */
		passwordGeneratorCalculatePassword(passGenerator, nextPasswordIndex, (char *) generatorPassword);

		if (NULL != outputFD) {
			fprintf(outputFD, "%s\t", firstPassword);
		}

		/* calc next hash */
		CHECK(0 != hashFunc(generatorPassword, strlen((char *) generatorPassword), currHash));

		if (NULL != outputFD) {
			binary2hexa(currHash, hashLen, currHashStr, sizeof(currHashStr));
			fprintf(outputFD, "%s\t", currHashStr);
		}

	}

	TRACE_FPRINTF((stderr, "pass: %s, hash: %lu (final)\n", generatorPassword, *((ulong_t *) currHash)));

	/* copy result hash to user */
	memcpy(hashBuf, currHash, MIN(hashBufLen, sizeof(currHash)));

	/* If we were asked to, supply the password as well. The string is always null terminated */
	if (NULL != passwordOut) {
		SAFE_STRNCPY((char *) passwordOut, (char *) generatorPassword, passwordOutLen);
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


bool_t RT_generate(	passwordEnumerator_t * passEnumerator,
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
	CHECK(NULL != enumeratorPassword);

	CHECK(NULL != hashFunc);
	CHECK(NULL != hashTableFilePrefix);

	if (0 == rainbowChainLen) {
		TRACE("exhaustive search mode");
	}
	else {
		CHECK(NULL != generatorPassword);
		CHECK(NULL != passGenerator);
	}

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
		/*!rainbowConfig->seeds[chainIndex] = getRandomULong(); !*/
		rainbowConfig->seeds[chainIndex] = 0;
	}

	/* dump to disk (will remain in mem until the ht is closed) */
	CHECK(DEHT_STATUS_FAIL != DEHT_writeUserBytes(ht));


	chainHashLen = getHashFunDigestLength(hashFunc);

	for (chainIndex = 1;  passwordEnumeratorCalculateNextPassword(passEnumerator);  ++chainIndex) {
		TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): working on chain %lu\n", __FILE__, __LINE__, __FUNCTION__, chainIndex));

		CHECK(buildChain(FALSE,
				 rainbowConfig->seeds, rainbowConfig->chainLength,
				 hashFunc,
			 	 passGenerator, (byte_t *) generatorPassword,
				 (byte_t *) enumeratorPassword, strlen(enumeratorPassword),
				 chainHash, sizeof(chainHash),
				 NULL, 0,
				 NULL));

		TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): inserting password \"%s\"\n", __FILE__, __LINE__, __FUNCTION__, enumeratorPassword));

		/* Insert resulting hash as the key and the initial password as the value */
		/* Inserting with terminating null due to the limited DEHT interface */
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
	/*! TODO: CLEAN DEHT FILES ON ERROR? !*/
	if (NULL != ht) {
		lock_DEHT_files(ht);
		ht = NULL;
	}

	TRACE_FUNC_EXIT();

	return ret;
}


RainbowTable_t * RT_open(
			const passwordGenerator_t * passGenerator,
			char * generatorPassword,

			const char * hashTableFilePrefix,
			bool_t enableFirstBlockCache,
			bool_t enableLastBlockCache)
{
	RainbowTable_t * ret = NULL;

	RainbowTable_t * self = NULL;

	ulong_t userBytesSize = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != hashTableFilePrefix);

	self = (RainbowTable_t *) malloc(sizeof(RainbowTable_t));
	CHECK(NULL != self);

	/* init */
	memset(self, 0, sizeof(RainbowTable_t));

	self->passGenerator = passGenerator;
	self->generatorPassword = (byte_t *) generatorPassword;

	self->hashTable = load_DEHT_from_files(hashTableFilePrefix, 
			       DEHT_keyToTableIndexHasher, DEHT_keyToValidationKeyHasher64);
	CHECK(NULL != self->hashTable);

	if (enableFirstBlockCache) {
		CHECK(DEHT_STATUS_FAIL != read_DEHT_pointers_table(self->hashTable));
	}
	if (enableLastBlockCache) {
		CHECK(DEHT_STATUS_FAIL != calc_DEHT_last_block_per_bucket(self->hashTable));
	}

	/* find the hash type */
	self->hashFunc = getHashFunFromName(self->hashTable->header.sDictionaryName);
	CHECK(NULL != self->hashFunc);

	/* load user bytes buffer */
	CHECK(DEHT_STATUS_FAIL != DEHT_readUserBytes(self->hashTable, (void **) &(self->config), &userBytesSize));
	CHECK(NULL != self->config);
	CHECK(userBytesSize == sizeof(RainbowTableConfig_t) + self->config->chainLength * sizeof(RainbowSeed_t));

	/* if chain length is 0, we will not require a password generator (otherwise, one is neccessary) */
	if (0 != self->config->chainLength) {
		CHECK(NULL != passGenerator);
		CHECK(NULL != generatorPassword);
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




void RT_close(RainbowTable_t * self)
{
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




bool_t RT_query(RainbowTable_t * self, 
		byte_t * hash, ulong_t hashLen,
		char * resPassword, ulong_t resPasswordLen)
{
	bool_t ret = FALSE;

	ulong_t j = 0;
	byte_t currHash[MAX_DIGEST_LEN];

	byte_t foundChainBeginPassword[MAX_PASSWORD_LEN];
	int foundChainBeginPasswordLen = 0;
	byte_t foundPassword[MAX_PASSWORD_LEN];


	TRACE_FUNC_ENTRY();

	CHECK(NULL != self);
	CHECK(NULL != hash);
	CHECK(NULL != resPassword);


	TRACE_FPRINTF((stderr, "target hash: %lu\n", *((ulong_t *) hash)));

	/* walk the chain corresponding to the given hash, trying to find a match */
	/* (j runs between 0 to chainLength + 1, since 0 means regular hashing, 1 is a chain of 1 cycle, etc...) */
	for (j = 0;  j < self->config->chainLength + 1;  ++j) {

		/* init currHash to the given hash */
		memcpy(currHash, hash, MIN(hashLen, sizeof(currHash)));

		/* in each step, we procceed one step down the chain (using the appropriate seed) */
		CHECK(buildChain(TRUE,
				 self->config->seeds, j,
				 self->hashFunc,
			 	 self->passGenerator, self->generatorPassword,
				 NULL, 0,
				 currHash, hashLen,
				 NULL, 0,
				 NULL));

		
		/* Now we have a hash that may be in the rainbow table - try looking for it */
		foundChainBeginPasswordLen = query_DEHT(self->hashTable, 
							currHash, hashLen, 
							foundChainBeginPassword, sizeof(foundChainBeginPassword));
		if (DEHT_STATUS_SUCCESS > foundChainBeginPasswordLen) {
			/* no dice - continue to the next chain depth */
			TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): no DEHT match for chain depth=%lu\n", __FILE__, __LINE__, __FUNCTION__, j));
			continue;
		}

		/* null terminate */		
		foundChainBeginPassword[MIN(foundChainBeginPasswordLen, sizeof(foundChainBeginPassword) - 1)] = 0x00;

		TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): foundChainBeginPassword=%s\n", __FILE__, __LINE__, __FUNCTION__, foundChainBeginPassword));

		SAFE_STRNCPY((char *) foundPassword, (char *) foundChainBeginPassword, sizeof(foundPassword));

		/* If we got here, we got a perliminary match (but it could still be a false alarm).
		   To find a possible password, step through the chain and look for a match */
		CHECK(buildChain(FALSE,
				 self->config->seeds, self->config->chainLength - j,
				 self->hashFunc,
			 	 self->passGenerator, self->generatorPassword,
				 foundChainBeginPassword, strlen((char *) foundChainBeginPassword),
				 currHash, hashLen,
				 foundPassword, sizeof(foundPassword),
				 NULL));

		if (0 != memcmp(currHash, hash, MIN(hashLen, sizeof(currHash)))) {
			TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): password \"%s\" was a false alarm (depth=%lu)\n", __FILE__, __LINE__, __FUNCTION__, foundPassword, j));
		}
		else {
			/* special case for chain length = 0 */
			if (0 == (self->config->chainLength - j)) {
				SAFE_STRNCPY((char *) resPassword, (char *) foundChainBeginPassword, resPasswordLen);
			}
			else {
				SAFE_STRNCPY((char *) resPassword, (char *) foundPassword, resPasswordLen);
			}
			TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): password \"%s\" matched! (depth=%lu)\n", __FILE__, __LINE__, __FUNCTION__, resPassword, j));

			ret = TRUE;
			goto LBL_CLEANUP;
		}

	}

	TRACE("exhaused search options. No match found.");
	goto LBL_ERROR;
	

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}















