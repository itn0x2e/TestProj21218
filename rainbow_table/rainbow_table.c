
#include "rainbow_table.h"
#include "../common/utils.h"
#include "../common/types.h"
#include "../common/misc.h"
#include "../password/password_enumerator.h"
#include "../common/DEHT.h"

int hashKeyIntoTableFunction(const unsigned char * key, int keySize, int tableSize)
{
	uint_t i = 0;
	byte_t res[sizeof(int)] = {0};
	int cksum = 0;

	for (i = 0;  i < keySize; ++i) {
		res[i] = res[i % sizeof(res)] ^ key[i];
	}

	cksum = *((int *) res);

	return (cksum % tableSize);
}

int hashKeyforEfficientComparisonFunction64(const unsigned char * key,int keySize, unsigned char * resBuf)
{
	uint_t i = 0;

	for (i = 0;  i < keySize; ++i) {
		resBuf[i] = resBuf[i % 8] ^ key[i];
	}

	return 0;
}


bool_t createRainbowTable(
			passwordEnumerator_t * passEnumerator,
			char * generatedPassword,

			BasicHashFunctionPtr hashFunc,

			ulong_t rainbowChainLen,

			char * hashTableFilePrefix,
			ulong_t nHashTableEntries,
			ulong_t nPairsPerBlock,
			bool_t enableFirstBlockCache,
			bool_t enableLastBlockCache)
{
	bool_t ret = FALSE;

	DEHT * ht = NULL;

	ulong_t chainIndex = 0;
	char firstPass[1024] = {0};

	TRACE_FUNC_ENTRY();

	CHECK(NULL != passEnumerator);
	CHECK(NULL != hashFunc);
	CHECK(NULL != hashTableFilePrefix);

	ht = create_empty_DEHT(hashTableFilePrefix, hashKeyIntoTableFunction, hashKeyforEfficientComparisonFunction64, getNameFromHashFun(hashFunc), nHashTableEntries, nPairsPerBlock, 8);
	CHECK(NULL != ht);

	if (enableFirstBlockCache) {
		CHECK(DEHT_STATUS_FAIL != read_DEHT_pointers_table(ht));
	}
	if (enableLastBlockCache) {
		CHECK(DEHT_STATUS_FAIL != calc_DEHT_last_block_per_bucket(ht));
	}


	/* generate seeds, store in ht */
	/*! TODO !*/


	while(passwordEnumeratorCalculateNextPassword(passEnumerator)) {
		
		/* save for later */
		SAFE_STRNCPY(firstPass, generatedPassword, sizeof(firstPass));

		for (chainIndex = 0; chainIndex < rainbowChainLen; ++chainIndex) {
				
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


