
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "../../common/utils.h"
#include "../DEHT.h"
#include "../hash_funcs.h"


#define TABLE_FILE_PREFIX "deht_test"
#define TABLE_DICT_NAME "test_dict"
#define TABLE_SIZE (50 * 1000 * 1000)
#define BLOCK_SIZE (1000)
#define USER_BYTES (1337)



/*
 * Real world setup
 */

#define KEY_SIZE (8)
#define TABLE_INDEX_FUNC (DEHT_keyToTableIndexHasher)
#define VALID_KEY_FUNC (DEHT_keyToValidationKeyHasher64)

/*
#define KEY_SIZE (32)
#define TABLE_INDEX_FUNC (DEHT_keyToTableIndexHasher)
#define VALID_KEY_FUNC (DEHT_keyToValidationKeyHasher128)
*/

/*
 * DEBUG config - simplifies collision creation and post-mortem investigation
 */
/*
#define KEY_SIZE (100)
#define TABLE_INDEX_FUNC (hashKeyIntoTableFunction)
#define VALID_KEY_FUNC (hashKeyforEfficientComparisonFunction)
*/


#define TORTURE_ELEM_COUNT (5000)
/* #define TORTURE_ELEM_COUNT (BLOCK_SIZE - 1) */
/* #define TORTURE_ELEM_COUNT (BLOCK_SIZE * 20) */


#define TABLE_TORTURE_RATIO (255)


typedef struct TestParams_s {
	ulong_t testDepth;
	char * keyFormatStr;
	char * dataFormatStr;
} TestParams_t;


typedef bool_t (* BucketTesterFunc_t) (DEHT * ht, byte_t bucketId, TestParams_t * params);




int hashKeyIntoTableFunction(const unsigned char * key, int keySize, int tableSize)
{
	int cksum = 0;

	/* to simplify collision creation, the buckets are chosen only by the first char */
	cksum = key[0] * (TABLE_SIZE / 255);
	return (cksum  % tableSize);


}

int hashKeyforEfficientComparisonFunction(const unsigned char * key,int keySize, unsigned char * resBuf)
{
	memcpy(resBuf, key, keySize);

	return 0;
}






bool_t createTable(DEHT ** ht, bool_t enableFirstBlockCache, bool_t enableLastBlockCache)
{
	bool_t ret = FALSE;
	
	CHECK(NULL != ht);

	*ht = create_empty_DEHT(TABLE_FILE_PREFIX, TABLE_INDEX_FUNC, VALID_KEY_FUNC, 
			       TABLE_DICT_NAME,
				TABLE_SIZE, BLOCK_SIZE, KEY_SIZE, USER_BYTES);
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

	*ht = load_DEHT_from_files(TABLE_FILE_PREFIX, TABLE_INDEX_FUNC, VALID_KEY_FUNC);
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



bool_t createAndCloseTable(void)
{
	bool_t ret = FALSE;
	
	DEHT * ht = NULL;

	ht = create_empty_DEHT(TABLE_FILE_PREFIX, TABLE_INDEX_FUNC, VALID_KEY_FUNC, 
			       TABLE_DICT_NAME,
				TABLE_SIZE, BLOCK_SIZE, KEY_SIZE, USER_BYTES);

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

	ht = load_DEHT_from_files(TABLE_FILE_PREFIX, TABLE_INDEX_FUNC, VALID_KEY_FUNC);
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



bool_t removeKeyFile(void)
{
	return (0 == remove(TABLE_FILE_PREFIX ".key"));
}

bool_t removeDataFile(void)
{
	return (0 == remove(TABLE_FILE_PREFIX ".data"));
}


bool_t removeFiles(void)
{
	bool_t ret = TRUE;

	/* try doing both anyway */	
	ret &= removeKeyFile();
	ret &= removeDataFile();

	return ret;
}


bool_t testCreateTableFileStates(void)
{
	bool_t ret = FALSE;

	TRACE_FUNC_ENTRY();

	/* begin from a clean state */
	(void) removeFiles();

	printf(">>>>>> creating a table with no prior files. Should create successfully\n");
	CHECK(createAndCloseTable());

	printf(">>>>>> creating a table with both prior files. Should fail miserably\n");
	CHECK(!createAndCloseTable());

	(void) removeDataFile();
	printf(">>>>>> creating a table with key file existing. Should fail miserably\n");
	CHECK(!createAndCloseTable());


	/* cleanup and state reconstruction */
	(void) removeFiles();
	CHECK(createAndCloseTable());

	(void) removeKeyFile();	
	printf(">>>>>> creating a table with data file existing. Should fail miserably\n");
	CHECK(!createAndCloseTable());

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}


bool_t testOpenTableFileStates(void)
{
	bool_t ret = FALSE;

	TRACE_FUNC_ENTRY();

	/* begin from a clean state */
	(void) removeFiles();

	printf(">>>>>> opening a table with no prior files. Should fail miserably\n");
	CHECK(!openAndCloseTable());


	CHECK(createAndCloseTable());
	printf(">>>>>> opening a table with both prior files. Should create successfully\n");
	CHECK(openAndCloseTable());

	(void) removeDataFile();
	printf(">>>>>> creating a table without data file. Should fail miserably\n");
	CHECK(!openAndCloseTable());

	/* cleanup and state reconstruction */
	(void) removeFiles();
	CHECK(createAndCloseTable());

	(void) removeKeyFile();	
	printf(">>>>>> creating a table with no key file. Should fail miserably\n");
	CHECK(!openAndCloseTable());

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}


bool_t testUserBytes(void)
{
	bool_t ret = FALSE;
	
	DEHT * ht = NULL;
	byte_t testData[USER_BYTES];
	byte_t * userBytes = NULL;
	ulong_t userBytesSize = 0;
	FILE * fd = NULL;


	/* start from a normal state */
	(void) removeFiles();

	ht = create_empty_DEHT(TABLE_FILE_PREFIX, TABLE_INDEX_FUNC, VALID_KEY_FUNC, 
			       TABLE_DICT_NAME,
				TABLE_SIZE, BLOCK_SIZE, KEY_SIZE, USER_BYTES);
	CHECK(NULL != ht);

	CHECK(DEHT_getUserBytes(ht, &userBytes, &userBytesSize));
	CHECK(NULL != userBytes);
	CHECK(USER_BYTES == userBytesSize);
	
	fd = fopen("/dev/urandom", "rb");
	CHECK(1 == fread(testData, sizeof(testData), 1, fd));


	/* copy test data to the buffer */
	memcpy(userBytes, testData, sizeof(testData));

	lock_DEHT_files(ht);
	ht = load_DEHT_from_files(TABLE_FILE_PREFIX, TABLE_INDEX_FUNC, VALID_KEY_FUNC);
	CHECK(NULL != ht);

	CHECK(DEHT_getUserBytes(ht, &userBytes, &userBytesSize));
	CHECK(NULL != userBytes);
	CHECK(USER_BYTES == userBytesSize);

	CHECK(0 == memcmp(userBytes, testData, sizeof(testData)));	

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:
	FCLOSE(fd);

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

	/*! These tests were meant for collision testing. They are of little importance given the proper hashing functions !*/
	for (bucketId = 0; bucketId < TABLE_TORTURE_RATIO; ++bucketId) {
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
		TRACE_FPRINTF((stdout, "%s: %d/%d, %lu/%lu\r", __FUNCTION__, bucketId + 1, ht->header.numEntriesInHashTable, elemCount+1, params->testDepth));


		snprintf(key+1, sizeof(key) - 1, params->keyFormatStr, elemCount);
		snprintf(data, sizeof(data), params->dataFormatStr, elemCount);

		CHECK(DEHT_STATUS_SUCCESS == add_DEHT(ht, key, sizeof(key), data, sizeof(data)));
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();
	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): last key was: %s\n", __FILE__, __LINE__, __FUNCTION__, key));

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
		TRACE_FPRINTF((stdout, "%s: %d/%d, %lu/%lu\r", __FUNCTION__, bucketId + 1, ht->header.numEntriesInHashTable, elemCount+1, params->testDepth));

		snprintf(key+1, sizeof(key) - 1, params->keyFormatStr, elemCount);
		snprintf(data, sizeof(data), params->dataFormatStr, elemCount);

		CHECK(DEHT_STATUS_NOT_NEEDED == insert_uniquely_DEHT(ht, key, sizeof(key), data, sizeof(data)));
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();
	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): last key was: %s\n", __FILE__, __LINE__, __FUNCTION__, key));

LBL_CLEANUP:
	return ret;
}



bool_t massiveQueryBucket(DEHT * ht, byte_t bucketId, TestParams_t * params)
{
	bool_t ret = FALSE;
	
	ulong_t elemCount = 0;
	byte_t key[100] = {0};
	byte_t expectedData[100] = {0};
	byte_t tempData[200] = {0};

	TRACE_FUNC_ENTRY();

	CHECK(NULL != ht);
	CHECK(NULL != params);

	key[0] = 0x30 + bucketId;

	/* in each round, we build the corresponding string for that value. they will all collide predictably, because the key
	   hash func only depends on the first byte */
	for (elemCount = 0; elemCount < params->testDepth; ++elemCount) {
		TRACE_FPRINTF((stdout, "%s: %d/%d, %lu/%lu\r", __FUNCTION__, bucketId + 1, ht->header.numEntriesInHashTable, elemCount+1, params->testDepth));

		snprintf(key+1, sizeof(key) - 1, params->keyFormatStr, elemCount);
		snprintf(expectedData, sizeof(expectedData), params->dataFormatStr, elemCount);

		CHECK(0 < query_DEHT(ht, key, sizeof(key), tempData, sizeof(tempData)));
		CHECK(0 == memcmp(tempData, expectedData, sizeof(expectedData)));
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();
	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): last key was: %s\n", __FILE__, __LINE__, __FUNCTION__, key));
	printf("expectedData=%s, tempData=%s\n", expectedData, tempData);

LBL_CLEANUP:
	return ret;
}




bool_t tortureTable(DEHT * ht, uint_t state)
{
	bool_t ret = FALSE;

	TestParams_t params = {0};
	time_t beginTime;
	time_t endTime;


	TRACE_FUNC_ENTRY();
	CHECK(NULL != ht);


	params.testDepth = TORTURE_ELEM_COUNT;
	params.keyFormatStr = "this_is_my_key_0123456789ABCDEF0123456789ABCDEF %lu";

	switch (state) {
	case 0:

		printf("state=0: assuming table is empty. doing many inserts, followed by queries and then updates\n");

		params.dataFormatStr = "this_is_my_data_0123456789ABCDEF0123456789ABCDEF %lu";

		beginTime = time(NULL);
	
		CHECK(testEntireTable(ht, massiveInsertIntoBucket, &params));
		printf("\nmassiveInsertIntoBucket passed\n\n");

		endTime = time(NULL);
		printf("\n\n>>> TIME: inserts took %lu secs\n", (ulong_t) (endTime - beginTime));

		printf("cotinuing to state=2 - many queries and updates\n");
		goto LBL_CASE_2;


	case 1:
		params.dataFormatStr = "this_is_my_new_data_0123456789ABCDEF0123456789ABCDEF %lu";

		printf("state=1: assuming table is filled with state=1 values. doing many queries, then many updates to state=0 values\n");

		beginTime = time(NULL);

		CHECK(testEntireTable(ht, massiveQueryBucket, &params));
		printf("\nmassiveQueryBucket passed\n\n");

		endTime = time(NULL);
		printf("\n\n>>> TIME: queries took %lu secs\n", (ulong_t) (endTime - beginTime));


		params.dataFormatStr = "this_is_my_data_0123456789ABCDEF0123456789ABCDEF %lu";


		beginTime = endTime;

		CHECK(testEntireTable(ht, massiveUpdateBucket, &params));
		printf("\nmassiveUpdateBucket passed\n");

		endTime = time(NULL);
		printf("\n\n>>> TIME: updates took %lu secs\n", (ulong_t) (endTime - beginTime));


		beginTime = endTime;

		/* test the update */
		CHECK(testEntireTable(ht, massiveQueryBucket, &params));
		printf("\nmassiveQueryBucket after update passed\n");

		endTime = time(NULL);
		printf("\n\n>>> TIME: queries took %lu secs\n", (ulong_t) (endTime - beginTime));

		break;

	case 2:
	LBL_CASE_2:

		printf("state=2: assuming table is filled with state=0 values. doing many queries, then many updates to state=1 values\n");

		params.dataFormatStr = "this_is_my_data_0123456789ABCDEF0123456789ABCDEF %lu";

		beginTime = time(NULL);

		CHECK(testEntireTable(ht, massiveQueryBucket, &params));
		printf("\nmassiveQueryBucket passed\n\n");

		endTime = time(NULL);
		printf("\n\n>>> TIME: queries took %lu secs\n", (ulong_t) (endTime - beginTime));
		
		beginTime = endTime;

		params.dataFormatStr = "this_is_my_new_data_0123456789ABCDEF0123456789ABCDEF %lu";

		CHECK(testEntireTable(ht, massiveUpdateBucket, &params));
		printf("\nmassiveUpdateBucket passed\n");

		endTime = time(NULL);
		printf("\n\n>>> TIME: updates took %lu secs\n", (ulong_t) (endTime - beginTime));

		beginTime = endTime;

		/* test the update */
		CHECK(testEntireTable(ht, massiveQueryBucket, &params));
		printf("\nmassiveQueryBucket after update passed\n");

		endTime = time(NULL);
		printf("\n\n>>> TIME: queries took %lu secs\n", (ulong_t) (endTime - beginTime));

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

	time_t beginTime;
	time_t endTime;

	TRACE_FUNC_ENTRY();

	printf("\n\ntestCreatedTable started\n");

	beginTime = time(NULL);
			
	CHECK(createTable(&ht, enableFirstBlockCache, enableLastBlockCache));

	endTime = time(NULL);

	printf("\n\n>>> TIME: init took %lu secs with settings: enableFirstBlockCache=%s, enableLastBlockCache=%s\n\n", (ulong_t) (endTime - beginTime), enableFirstBlockCache ? "True" : "False", enableLastBlockCache ? "True" : "False");


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


	CHECK(testCreateTableFileStates());
	printf(">>> testCreateTableFileStates - passed.\n");

	CHECK(testOpenTableFileStates());
	printf(">>> testOpenTableFileStates - passed.\n");

	CHECK(testUserBytes());
	printf(">>> testUserBytes - passed.\n");

	for (enableFirstBlockCache = 0; enableFirstBlockCache < 2; ++enableFirstBlockCache) {
		for (enableLastBlockCache = 0; enableLastBlockCache < 2; ++enableLastBlockCache) {
			printf("\n>>> testing table for enableFirstBlockCache=%s, enableLastBlockCache=%s\n", enableFirstBlockCache ? "True" : "False", enableLastBlockCache ? "True" : "False");

			(void) removeFiles();

			CHECK(testCreatedTable(enableFirstBlockCache, enableLastBlockCache));
			printf(">>>>>>>> testCreatedTable - passed.\n");
			CHECK(testOpenedTable(enableFirstBlockCache, enableLastBlockCache));
			printf(">>>>>>>> testOpenedTable - passed.\n");

		}

	}

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
