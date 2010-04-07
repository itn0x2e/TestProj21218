
#include <time.h>
#include <stdio.h>
#include "../common/utils.h"
#include "DEHT.h"



typedef struct TestParams_s {
	ulong_t testDepth;
	char * keyFormatStr;
	char * dataFormatStr;
} TestParams_t;


typedef bool_t (* BucketTesterFunc_t) (DEHT * ht, byte_t bucketId, TestParams_t * params);




int hashKeyIntoTableFunction(const unsigned char * key, int keySize, int tableSize)
{
	int cksum = 0;
/*	int i = 0;
	for (i = 0; i < keySize; ++i) {
		cksum = cksum ^ key[i];
	}
*/
	/* to simplify collision creation, the buckets are chosen only by the first char */
	cksum = key[0];
	return (cksum  % tableSize);


}

int hashKeyforEfficientComparisonFunction(const unsigned char * key,int keySize, unsigned char * resBuf)
{
	memcpy(resBuf, key, keySize);
}




bool_t createEmptyTable(DEHT ** ht, bool_t enableFirstBlockCache, bool_t enableLastBlockCache)
{
	bool_t ret = FALSE;
	
	CHECK(NULL != ht);

	*ht = create_empty_DEHT("test", hashKeyIntoTableFunction, hashKeyforEfficientComparisonFunction, 
			       "test_dict",
				255, 5, 100, 0);
	CHECK(NULL != *ht);

	if (enableFirstBlockCache) {
		CHECK(DEHT_STATUS_FAIL != read_DEHT_pointers_table(*ht));
	}
	if (enableLastBlockCache) {
		CHECK(DEHT_STATUS_FAIL != calc_DEHT_last_block_per_bucket(*ht));
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:
	
	return ret;
}


bool_t openTable(DEHT ** ht, bool_t enableFirstBlockCache, bool_t enableLastBlockCache)
{
	bool_t ret = FALSE;
	
	CHECK(NULL != ht);

	*ht = load_DEHT_from_files("test", hashKeyIntoTableFunction, hashKeyforEfficientComparisonFunction);
	CHECK(NULL != *ht);

	if (enableFirstBlockCache) {
		CHECK(DEHT_STATUS_FAIL != read_DEHT_pointers_table(*ht));
	}
	if (enableLastBlockCache) {
		CHECK(DEHT_STATUS_FAIL != calc_DEHT_last_block_per_bucket(*ht));
	}


	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:
	
	return ret;
}



bool_t createAndCloseEmptyTable(void)
{
	bool_t ret = FALSE;
	
	DEHT * ht = NULL;

	ht = create_empty_DEHT("test", hashKeyIntoTableFunction, hashKeyforEfficientComparisonFunction, 
			       "test_dict",
				255, 5, 100, 0);

	CHECK(NULL != ht);

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:

	if (NULL != ht) {
		lock_DEHT_files(ht);
		ht = NULL;
	}
	
	return ret;
}


bool_t openAndCloseTable(void)
{
	bool_t ret = FALSE;
	
	DEHT * ht = NULL;

	ht = load_DEHT_from_files("test", hashKeyIntoTableFunction, hashKeyforEfficientComparisonFunction);
	CHECK(NULL != ht);

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:

	if (NULL != ht) {
		lock_DEHT_files(ht);
		ht = NULL;
	}
	
	return ret;
}






bool_t testEntireTable(DEHT * ht, BucketTesterFunc_t func, TestParams_t * params)
{
	bool_t ret = FALSE;
	byte_t bucketId = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != func);

	for (bucketId = 0; bucketId < ht->header.numEntriesInHashTable; ++bucketId) {
		CHECK(func(ht, bucketId, params));
	}
	
	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:

	return ret;
}



bool_t massiveInsertIntoBucket(DEHT * ht, byte_t bucketId, TestParams_t * params)
{
	bool_t ret = FALSE;
	
	ulong_t elemCount = 0;
	byte_t key[100] = {0};
	byte_t data[100] = {0};

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != params);

	key[0] = 0x30 + bucketId;

	/* in each round, we build the corresponding string for that value. they will all collide predictably, because the key
	   hash func only depends on the first byte */
	for (elemCount = 0; elemCount < params->testDepth; ++elemCount) {
		TRACE_FPRINTF(stdout, "%s: %d/%d, %lu/%lu\r", __FUNCTION__, bucketId + 1, ht->header.numEntriesInHashTable, elemCount+1, params->testDepth);

		snprintf(key+1, sizeof(key) - 1, params->keyFormatStr, elemCount);
		snprintf(data, sizeof(data), params->dataFormatStr, elemCount);

		CHECK(DEHT_STATUS_SUCCESS == add_DEHT(ht, key, sizeof(key), data, sizeof(data)));
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	return ret;
}


bool_t massiveUpdateBucket(DEHT * ht, byte_t bucketId, TestParams_t * params)
{
	bool_t ret = FALSE;
	
	ulong_t elemCount = 0;
	byte_t key[100] = {0};
	byte_t data[100] = {0};


	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != params);


	key[0] = 0x30 + bucketId;

	/* in each round, we build the corresponding string for that value. they will all collide predictably, because the key
	   hash func only depends on the first byte */
	for (elemCount = 0; elemCount < params->testDepth; ++elemCount) {
		TRACE_FPRINTF(stdout, "%s: %d/%d, %lu/%lu\r", __FUNCTION__, bucketId + 1, ht->header.numEntriesInHashTable, elemCount+1, params->testDepth);

		snprintf(key+1, sizeof(key) - 1, params->keyFormatStr, elemCount);
		snprintf(data, sizeof(data), params->dataFormatStr, elemCount);

		CHECK(DEHT_STATUS_NOT_NEEDED == insert_uniquely_DEHT(ht, key, sizeof(key), data, sizeof(data)));
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	return ret;
}



bool_t massiveQueryBucket(DEHT * ht, byte_t bucketId, TestParams_t * params)
{
	bool_t ret = FALSE;
	
	ulong_t elemCount = 0;
	byte_t key[80] = {0};
	byte_t expectedData[100] = {0};
	byte_t tempData[200] = {0};

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != params);

	key[0] = 0x30 + bucketId;

	/* in each round, we build the corresponding string for that value. they will all collide predictably, because the key
	   hash func only depends on the first byte */
	for (elemCount = 0; elemCount < params->testDepth; ++elemCount) {
		TRACE_FPRINTF(stdout, "%s: %d/%d, %lu/%lu\r", __FUNCTION__, bucketId + 1, ht->header.numEntriesInHashTable, elemCount+1, params->testDepth);

		snprintf(key+1, sizeof(key) - 1, params->keyFormatStr, elemCount);
		snprintf(expectedData, sizeof(expectedData), params->dataFormatStr, elemCount);

		CHECK(0 < query_DEHT(ht, key, sizeof(key), tempData, sizeof(tempData)));
		CHECK(0 == memcmp(tempData, expectedData, strlen(expectedData)));
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	printf("expectedData=%s, tempData=%s\n", expectedData, tempData);

	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	return ret;
}




bool_t tortureTable(DEHT * ht, uint_t state)
{
	bool_t ret = FALSE;

	TestParams_t params = {0};

	TRACE_FUNC_ENTRY();
	CHECK(NULL != ht);


	params.testDepth = 4;
	params.keyFormatStr = "this_is_my_key_0123456789ABCDEF0123456789ABCDEF %lu";

	switch (state) {
	case 0:

		printf("state=0: assuming table is empty. doing many inserts, followed by queries and then updates\n");

		params.dataFormatStr = "this_is_my_data_0123456789ABCDEF0123456789ABCDEF %lu";

		CHECK(testEntireTable(ht, massiveInsertIntoBucket, &params));
		printf("\nmassiveInsertIntoBucket passed\n\n");

		printf("cotinuing to state=2 - many queries and updates\n");
		goto LBL_CASE_2;


	case 1:
	LBL_CASE_1:
		params.dataFormatStr = "this_is_my_new_data_0123456789ABCDEF0123456789ABCDEF %lu";

		printf("state=1: assuming table is filled with state=1 values. doing many queries, then many updates to state=0 values\n");

		CHECK(testEntireTable(ht, massiveQueryBucket, &params));
		printf("\nmassiveQueryBucket passed\n\n");


		params.dataFormatStr = "this_is_my_data_0123456789ABCDEF0123456789ABCDEF %lu";

		CHECK(testEntireTable(ht, massiveUpdateBucket, &params));
		printf("\nmassiveUpdateBucket passed\n");

		/* test the update */
		CHECK(testEntireTable(ht, massiveQueryBucket, &params));
		printf("\nmassiveQueryBucket after update passed\n");

		break;

	case 2:
	LBL_CASE_2:

		printf("state=2: assuming table is filled with state=0 values. doing many queries, then many updates to state=1 values\n");

		params.dataFormatStr = "this_is_my_data_0123456789ABCDEF0123456789ABCDEF %lu";

		CHECK(testEntireTable(ht, massiveQueryBucket, &params));
		printf("\nmassiveQueryBucket passed\n\n");


		params.dataFormatStr = "this_is_my_new_data_0123456789ABCDEF0123456789ABCDEF %lu";

		CHECK(testEntireTable(ht, massiveUpdateBucket, &params));
		printf("\nmassiveUpdateBucket passed\n");

		/* test the update */
		CHECK(testEntireTable(ht, massiveQueryBucket, &params));
		printf("\nmassiveQueryBucket after update passed\n");


		break;

	default:
		FAIL("bad state requested");		

	}

	ret = TRUE;
	goto LBL_CLEANUP;		

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	return ret;
}


bool_t testCreatedTable(bool_t enableFirstBlockCache, bool_t enableLastBlockCache)
{
	bool_t ret = FALSE;
	DEHT * ht = NULL;

	TRACE_FUNC_ENTRY();

	printf("\n\ntestCreatedTable started\n");

	CHECK(createEmptyTable(&ht, enableFirstBlockCache, enableLastBlockCache));

	CHECK(tortureTable(ht, 0));

	printf("testCreatedTable passed!\n");

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	if (NULL != ht) {
		lock_DEHT_files(ht);
		ht = NULL;
	}

	TRACE_FUNC_EXIT();
	return ret;
}


bool_t testOpenedTable(bool_t enableFirstBlockCache, bool_t enableLastBlockCache)
{
	bool_t ret = FALSE;
	DEHT * ht = NULL;

	TRACE_FUNC_ENTRY();

	printf("\n\ntestOpenedTable started\n");

	CHECK(openTable(&ht, enableFirstBlockCache, enableLastBlockCache));

	CHECK(tortureTable(ht, 1));

	printf("testOpenedTable passed for state=1\n");
	printf("\nclosing and re-opening\n");
	lock_DEHT_files(ht);
	ht = NULL;

	CHECK(openTable(&ht, enableFirstBlockCache, enableLastBlockCache));

	CHECK(tortureTable(ht, 2));

	printf("testOpenedTable passed for state=2\n");

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	if (NULL != ht) {
		lock_DEHT_files(ht);
		ht = NULL;
	}

	TRACE_FUNC_EXIT();
	return ret;
}




int main(void)
{
	int ret = -2;
	bool_t enableFirstBlockCache = FALSE;
	bool_t enableLastBlockCache = FALSE;

	DEHT * ht = NULL;

	time_t beginTime;
	time_t endTime;

	CHECK(createAndCloseEmptyTable());
	printf(">>> createAndCloseEmptyTable - passed.\n");

	CHECK(openAndCloseTable());
		printf(">>> openAndCloseTable - passed.\n");

	for (enableFirstBlockCache = 0; enableFirstBlockCache < 2; ++enableFirstBlockCache) {
		for (enableLastBlockCache = 0; enableLastBlockCache < 2; ++enableLastBlockCache) {
			printf("\n>>> testing table for enableFirstBlockCache=%s, enableLastBlockCache=%s\n", enableFirstBlockCache ? "True" : "False", enableLastBlockCache ? "True" : "False");

			beginTime = time(NULL);

			CHECK(testCreatedTable(enableFirstBlockCache, enableLastBlockCache));
			printf(">>>>>>>> testCreatedTable - passed.\n");
			CHECK(testOpenedTable(enableFirstBlockCache, enableLastBlockCache));
			printf(">>>>>>>> testOpenedTable - passed.\n");

			endTime = time(NULL);

			printf("\n\n>>> tests for enableFirstBlockCache=%s, enableLastBlockCache=%s took %lu secs.\n\n", enableFirstBlockCache ? "True" : "False", enableLastBlockCache ? "True" : "False", (ulong_t) (endTime - beginTime));
		}

	}

	/*
	CHECK(createEmptyTable());
	printf("createEmptyTable passed\n");
	CHECK(openEmptyTable());
	printf("openEmptyTable passed\n");
*/
	printf("\n>>> All tests passed!\n");

	ret = 0;
	goto LBL_CLEANUP;

LBL_ERROR:
	printf("\n\n>>> last test failed!\n");
	TRACE_FUNC_ERROR();
	ret = -1;

LBL_CLEANUP:

	return ret;
}
