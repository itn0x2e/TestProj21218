

#include "../common/constants.h"
#include "../common/utils.h"
#include "../common/misc.h"
#include "../common/io.h"
#include "../common/types.h"
#include "../DEHT/DEHT.h"
#include "../DEHT/hash_funcs.h"
#include "rainbow_table.h"




typedef struct htEnumerationParams_s {
	RainbowTable_t * rt;

	FILE * fdPasswords;
	FILE * fdChains;
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


bool_t printAndCalcChain(RainbowTable_t * rt, FILE * outputFD,
			   char * password, ulong_t passwordLen,
			   byte_t * hashOut, ulong_t hashOutLen)
{
	bool_t ret = FALSE;

	byte_t verifiedHash[MAX_DIGEST_LEN];

	TRACE_FUNC_ENTRY();	

	CHECK(NULL != rt);
	CHECK(NULL != password);



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



static void hashTableEnumerationFunc(byte_t * key, ulong_t keySize, 
				     byte_t * data, ulong_t dataLen,
				     void * params)
{
	htEnumerationParams_t * enumerationParams = params;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != key);
	CHECK(NULL != data);
	CHECK(NULL != params);

	CHECK(NULL != enumerationParams->fdPasswords);
	CHECK(NULL != enumerationParams->fdChains);

	
	fprintf(enumerationParams->fdPasswords, "%s\n", data);

	fprintf(enumerationParams->fdChains, "%s\n", "would have printed the chain");

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
		     FALSE, FALSE);
	CHECK(NULL != rt);


	/* print seeds */	
	CHECK(printSeeds(rt, seedsAndPasswordsFd));

	/* set up enumeration params */
	memset(&enumerationParams, 0, sizeof(enumerationParams));
	enumerationParams.rt = rt;
	enumerationParams.fdPasswords = seedsAndPasswordsFd;
	enumerationParams.fdChains = chainsFd;

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
