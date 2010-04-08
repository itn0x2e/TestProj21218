
#include "rainbow_table.h"
#include "../common/utils.h"
#include "../common/types.h"
#include "../common/misc.h"
#include "../password/password_enumerator.h"
#include "../common/DEHT.h"


typedef ulong_t RainbowSeedLen_t;


bool_t createRainbowTable(
			const passwordEnumerator_t * passEnumerator,
			const passwordGenerator_t * passwordGenerator;
			char * generatedPassword,

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
	ulong_t inChainIndex = 0;

	char firstPassword[1024] = {0};
	RainbowSeedLen_t * seeds = NULL;
	ulong_t userBytesSize = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != passEnumerator);
	CHECK(NULL != hashFunc);
	CHECK(NULL != hashTableFilePrefix);

	ht = create_empty_DEHT(hashTableFilePrefix, 
			       hashKeyIntoTableFunction, hashKeyforEfficientComparisonFunction64, 
			       getNameFromHashFun(hashFunc), 
			       nHashTableEntries, nPairsPerBlock, 
			       rainbowChainLen * sizeof(RainbowSeedLen_t));
	CHECK(NULL != ht);

	if (enableFirstBlockCache) {
		CHECK(DEHT_STATUS_FAIL != read_DEHT_pointers_table(ht));
	}
	if (enableLastBlockCache) {
		CHECK(DEHT_STATUS_FAIL != calc_DEHT_last_block_per_bucket(ht));
	}

	


	/* load user bytes buffer (even though it should be empty right now, this will allow us to use the table's buffer, which gets saved to disk automagically) */
	CHECK(readUserBytes(ht, &seeds, &userBytesSize));
	CHECK(NULL != seeds);
	CHECK(userBytesSize == rainbowChainLen * sizeof(RainbowSeedLen_t));
	/* generate seeds, store in ht */
	for (inChainIndex = 0;  inChainIndex < rainbowChainLen; ++inChainIndex) {
		seeds[inChainIndex] = getRandomULong();
	}
	/* dump to disk (will remain until the ht is closed) */
	CHECK(writeUserBytes(ht));


	for (chainIndex = 1; passwordEnumeratorCalculateNextPassword(passEnumerator), ++chainIndex) {
		TRACE_FPRINTF(stderr, "TRACE: %s:%d (%s): working on chain %d\r", __FILE__, __LINE__, __FUNCTION__, 
		
		/* save for later */
		SAFE_STRNCPY(firstPassword, generatedPassword, sizeof(firstPassword));

		for (inChainIndex = 0; inChainIndex < rainbowChainLen; ++inChainIndex) {
			
		}

	}
	
	
	


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


