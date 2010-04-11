

#include "../common/constants.h"
#include "../common/utils.h"
#include "../common/misc.h"
#include "../common/types.h"
#include "../DEHT/DEHT.h"
#include "../DEHT/hash_funcs.h"
#include "rainbow_table.h"




typedef struct htEnumerationParams_s {
	RainbowTable_t * rt;

	FILE * passwordsFd;
	FILE * chainsFd;
} htEnumerationParams_t;

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
	CHECK(DEHT_STATUS_FAIL != DEHT_getUserBytes(rt->hashTable, (byte_t **) &(rt->config), &userBytesSize));
	CHECK(NULL != rt->config);
	CHECK(userBytesSize == getConfigSize(rt->config->chainLength));


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


bool_t calcAndPrintChain(RainbowTable_t * rt, FILE * outputFd,
			   char * password, ulong_t passwordLen,
			   byte_t * hashOut, ulong_t hashOutLen)
{
	bool_t ret = FALSE;

	byte_t curHash[MAX_DIGEST_LEN];
	char hashStr[MAX_DIGEST_LEN * 2 + 1];
	int hashLen = 0;
	
	ulong_t j = 0;
	ulong_t numPossiblePasswords;

	TRACE_FUNC_ENTRY();	

	CHECK(NULL != rt);
	CHECK(NULL != outputFd);
	CHECK(NULL != password);
	CHECK(NULL != hashOut);
	
	numPossiblePasswords = passwordGeneratorGetSize(rt->passGenerator);

       /* first, print the initial password */
        fprintf(outputFd, "%s", password);
        
        /* calculate its hash */
        CHECK(NULL != rt->hashFunc);
        hashLen = cryptHash(rt->hashFunc, password, curHash);
        CHECK(0 != hashLen);
        
        /* print the hash (in hex) */
        binary2hexa(curHash, hashLen, hashStr, sizeof(hashStr));
        fprintf(outputFd, "\t%s", hashStr);


	for (j = 0; j < rt->config->chainLength; ++j) {
		/* k = pseudo-random-function with seed seed[j] and input curHash; */
		RainbowSeed_t k = pseudo_random_function(curHash, hashLen, rt->config->seeds[j]); /*! TODO: yet to be implemented */
	
		/* curHash = cryptographic-hash(get_kth_password_64b(k,S)); */
		k %= numPossiblePasswords;
		passwordGeneratorCalculatePassword(rt->passGenerator, k, rt->password);
		cryptHash(rt->hashFunc, rt->password, curHash);
	
		/* print appropriate password */
		fprintf(outputFd, "\t%s", rt->password);

		/* print corresponding hash */
		binary2hexa(curHash, hashLen, hashStr, sizeof(hashStr));
		fprintf(outputFd, "\t%s", hashStr);
	}

	/* copy result hash to user */
	memcpy(hashOut, curHash, MIN(sizeof(curHash), hashOutLen));

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;

}



static void hashTableEnumerationFunc(int bucketIndex,
				     byte_t * key, ulong_t keySize, 
				     byte_t * data, ulong_t dataLen,
				     void * params)
{
	htEnumerationParams_t * enumerationParams = params;

	byte_t verifiedHash[MAX_DIGEST_LEN];

	int verifiedBucketIndex = 0;
	byte_t verifiedValidationKey[BYTES_PER_KEY];

	TRACE_FUNC_ENTRY();

	CHECK(NULL != key);
	CHECK(NULL != data);
	CHECK(NULL != params);

	CHECK(NULL != enumerationParams->rt);
	CHECK(NULL != enumerationParams->passwordsFd);
	CHECK(NULL != enumerationParams->chainsFd);


	fprintf(enumerationParams->passwordsFd, "%s\n", data);

	CHECK(calcAndPrintChain(enumerationParams->rt, enumerationParams->chainsFd,
				(char *) data, dataLen,
				verifiedHash, sizeof(verifiedHash)));

	/* calculate this hash's corresponding table index */
	verifiedBucketIndex = DEHT_keyToTableIndexHasher(verifiedHash, getHashFunDigestLength(enumerationParams->rt->hashFunc), enumerationParams->rt->hashTable->header.numEntriesInHashTable);

	/* calculate this hash's validation key */
	(void) DEHT_keyToValidationKeyHasher64(verifiedHash, getHashFunDigestLength(enumerationParams->rt->hashFunc), verifiedValidationKey);


	if ((verifiedBucketIndex != bucketIndex) || (0 != memcmp(verifiedValidationKey, key, keySize))) {
		fprintf(enumerationParams->chainsFd, "\nError: when begin with %s get wrong chain", data);
	}

	fprintf(enumerationParams->chainsFd, "\n");

	goto LBL_CLEANUP;


LBL_ERROR:
	TRACE_FUNC_ERROR();
	
LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return;
}



bool_t RT_print(FILE * seedsAndPasswordsFd,
		FILE * chainsFd,

		const passwordGenerator_t * passGenerator,
		char * generatorPassword,
		ulong_t passwordMaxLen,
		const char * hashTableFilePrefix)
{
	bool_t ret = FALSE;

	RainbowTable_t * rt = NULL;

	htEnumerationParams_t enumerationParams;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != seedsAndPasswordsFd);
	CHECK(NULL != chainsFd);

	CHECK(NULL != hashTableFilePrefix);
	CHECK(NULL != passGenerator);
	CHECK(NULL != generatorPassword);


	/* Open rainbow table (all caches off, since we'll be doing sequencial access */
	rt = RT_open(passGenerator, generatorPassword, 
		     passwordMaxLen, hashTableFilePrefix, 
		     FALSE);
	CHECK(NULL != rt);


	/* print seeds */	
	CHECK(printSeeds(rt, seedsAndPasswordsFd));

	/* set up enumeration params */
	memset(&enumerationParams, 0, sizeof(enumerationParams));
	enumerationParams.rt = rt;
	enumerationParams.passwordsFd = seedsAndPasswordsFd;
	enumerationParams.chainsFd = chainsFd;


	/* write description into files */
	fprintf(seedsAndPasswordsFd, "Passwords:\n");

	/* scan all chains */	
	CHECK(DEHT_enumerate(rt->hashTable, hashTableEnumerationFunc, (void *) &enumerationParams));

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
