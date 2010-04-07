
#include <stdio.h>
#include "utils.h"
#include "DEHT.h"


typedef bool_t (* bucketTesterFunc_t) (DEHT * ht, byte_t bucketId, void * param);



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




bool_t createEmptyTable(DEHT ** ht)
{
	bool_t ret = FALSE;
	
	CHECK(NULL != ht);

	*ht = create_empty_DEHT("test", hashKeyIntoTableFunction, hashKeyforEfficientComparisonFunction, 
			       "test_dict",
				10, 5, 100);
	CHECK(NULL != *ht);

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:
	
	return ret;
}


bool_t openTable(DEHT ** ht)
{
	bool_t ret = FALSE;
	
	CHECK(NULL != ht);

	*ht = load_DEHT_from_files("test", hashKeyIntoTableFunction, hashKeyforEfficientComparisonFunction);
	CHECK(NULL != *ht);

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
				10, 5, 100);

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






bool_t testEntireTable(DEHT * ht, bucketTesterFunc_t func, void * param)
{
	bool_t ret = FALSE;
	byte_t bucketId = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != func);

	for (bucketId = 0; bucketId < ht->header.numEntriesInHashTable; ++bucketId) {
		CHECK(func(ht, bucketId, param));
	}
	
	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:

	return ret;
}



bool_t massiveInsertIntoBucket(DEHT * ht, byte_t bucketId, void * param)
{
	bool_t ret = FALSE;
	
	ulong_t elemCount = 0;
	byte_t key[80] = {0};
	byte_t data[] = "test data";

	ulong_t targetElemCount = 0;


	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != param);

	targetElemCount = *((ulong_t *) param);

	key[0] = 0x30 + bucketId;

	/* in each round, we build the corresponding string for that value. they will all collide predictably, because the key
	   hash func only depends on the first byte */
	for (elemCount = 0; elemCount < targetElemCount; ++elemCount) {
		printf("%s: %d/%d, %lu/%lu\r", __FUNCTION__, bucketId + 1, ht->header.numEntriesInHashTable, elemCount+1, targetElemCount);

		snprintf(key+1, sizeof(key) - 1, "baa_01234567890ABCDEF01234567890ABCDEF %lu", elemCount);
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


bool_t massiveUpdateBucket(DEHT * ht, byte_t bucketId, void * param)
{
	bool_t ret = FALSE;
	
	ulong_t elemCount = 0;
	byte_t key[80] = {0};
	byte_t data[] = "new test data - now with exta binary!";

	ulong_t targetElemCount = 0;


	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != param);

	targetElemCount = *((ulong_t *) param);

	key[0] = 0x30 + bucketId;

	/* in each round, we build the corresponding string for that value. they will all collide predictably, because the key
	   hash func only depends on the first byte */
	for (elemCount = 0; elemCount < targetElemCount; ++elemCount) {
		printf("%s: %d/%d, %lu/%lu\r", __FUNCTION__, bucketId + 1, ht->header.numEntriesInHashTable, elemCount+1, targetElemCount);

		snprintf(key+1, sizeof(key) - 1, "baa_01234567890ABCDEF01234567890ABCDEF %lu", elemCount);
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



bool_t massiveQueryBucket(DEHT * ht, byte_t bucketId, void * param)
{
	bool_t ret = FALSE;
	
	ulong_t elemCount = 0;
	byte_t key[80] = {0};
	byte_t expectedData[] = "test data";
	byte_t tempData[200] = {0};

	ulong_t targetElemCount = 0;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != param);

	targetElemCount = *((ulong_t *) param);

	key[0] = 0x30 + bucketId;

	/* in each round, we build the corresponding string for that value. they will all collide predictably, because the key
	   hash func only depends on the first byte */
	for (elemCount = 0; elemCount < targetElemCount; ++elemCount) {
		printf("%s: %d/%d, %lu/%lu\r", __FUNCTION__, bucketId + 1, ht->header.numEntriesInHashTable, elemCount+1, targetElemCount);

		snprintf(key+1, sizeof(key) - 1, "baa_01234567890ABCDEF01234567890ABCDEF %lu", elemCount);
		CHECK(0 < query_DEHT(ht, key, sizeof(key), tempData, sizeof(tempData)));
		CHECK(0 == memcmp(tempData, expectedData, strlen(expectedData)));
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	return ret;
}




bool_t tortureTable(DEHT * ht)
{
	bool_t ret = FALSE;

	ulong_t targetChainLen = 1337;

	TRACE_FUNC_ENTRY();
	CHECK(NULL != ht);

	CHECK(testEntireTable(ht, massiveInsertIntoBucket, &targetChainLen));
	printf("\nmassiveInsertIntoBucket passed\n\n");

	CHECK(testEntireTable(ht, massiveQueryBucket, &targetChainLen));
	printf("\nmassiveQueryBucket passed\n\n");

	CHECK(testEntireTable(ht, massiveUpdateBucket, &targetChainLen));
	printf("\nmassiveUpdateBucket passed\n\n\n");

	ret = TRUE;
	goto LBL_CLEANUP;		

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	return ret;
}


bool_t testCreatedTable(void)
{
	bool_t ret = FALSE;
	DEHT * ht = NULL;

	TRACE_FUNC_ENTRY();

	printf("testCreatedTable started\n");

	CHECK(createEmptyTable(&ht));

	CHECK(tortureTable(ht));

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




int main(void)
{
	int ret = -2;

	DEHT * ht = NULL;

	CHECK(createAndCloseEmptyTable());
	printf("createAndCloseEmptyTable - passed\n");

	CHECK(openAndCloseTable());
	printf("openAndCloseTable - passed\n");

	CHECK(testCreatedTable());


	/*
	CHECK(createEmptyTable());
	printf("createEmptyTable passed\n");
	CHECK(openEmptyTable());
	printf("openEmptyTable passed\n");
*/
	printf("\ndone.\n");

	ret = 0;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = -1;

LBL_CLEANUP:

	return ret;
}
