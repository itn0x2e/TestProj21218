#include <stdio.h>
#include <stdlib.h>
#include "../../common/misc.h"
#include "../../password/dictionary.h"
#include "../../password/password_generator.h"
#include "../../password/random_password_enumerator.h"
#include "../../password/all_password_enumerator.h"
#include "../rainbow_table.h"
#include "../../common/constants.h"


#define HASH_FUNC_NAME ("MD5")
#define RAINBOW_CHAIN_LEN (1)
#define HASH_TABLE_FILE_PREFIX ("rainbow_test")
#define HASH_TABLE_ENTRIES 100
#define HASH_TABLE_PAIRS_PER_BLOCK (100)


/*!
 * No longer relevant
!*/
/*!
#define FIRST_BLOCK_PTRS_CACHE_ENABLE (TRUE)
#define LAST_BLOCK_PTRS_CACHE_ENABLE (TRUE)
!*/




bool_t removeKeyFile(void)
{
	return (0 == remove("rainbow_test.key"));
}

bool_t removeDataFile(void)
{
	return (0 == remove("rainbow_test.data"));
}


bool_t removeFiles(void)
{
	bool_t ret = TRUE;

	/* try doing both anyway */	
	ret &= removeKeyFile();
	ret &= removeDataFile();

	return ret;
}


bool_t testCrackPassword(RainbowTable_t * rt, 
			 BasicHashFunctionPtr hashFunc, 
			 char * password)
{
	bool_t ret = FALSE;

	byte_t hash[MAX_DIGEST_LEN];
	int hashLen = 0;

	char crackedPassword[MAX_PASSWORD_LEN];
	byte_t crackedPasswordHash[MAX_DIGEST_LEN];

	TRACE_FUNC_ENTRY();
	
	CHECK(NULL != rt);
	CHECK(NULL != hashFunc);
	CHECK(NULL != password);

	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): testing for password=%s\n", __FILE__, __LINE__, __FUNCTION__, password));

	hashLen = hashFunc((byte_t *) password, strlen(password), hash);

	/* create hash */
	CHECK(0 != hashLen);

	CHECK(RT_query(rt, 
		       hash, hashLen,
		       crackedPassword, sizeof(crackedPassword)));

	CHECK(0 != hashFunc((byte_t *) crackedPassword, strlen(crackedPassword), crackedPasswordHash));

	CHECK(0 == memcmp(hash, crackedPasswordHash, hashLen));

	TRACE_FPRINTF((stderr, "TRACE: %s:%d (%s): cracked password=%s\n", __FILE__, __LINE__, __FUNCTION__, password));

	if (0 != strcmp(password, crackedPassword)) {
		printf(">>>>>> found a collision for password \"%s\", but it's a different password: %s\n", password, crackedPassword);
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	printf(">>>>>> problematic password: \"%s\"\n", password);
	TRACE_FUNC_ERROR();
	ret = FALSE;


LBL_CLEANUP:
	return ret;
}


bool_t testCrackAllPasswordsInRange(RainbowTable_t * rt, 
				    passwordGenerator_t * passGen, char * genPass)
{
	char passBuf[MAX_PASSWORD_LEN];
	bool_t ret = FALSE;

	BasicHashFunctionPtr hashFunc = NULL;
	
	allPasswordEnumerator_t passEnumerator;

	TRACE_FUNC_ENTRY();
	
	CHECK(NULL != rt);

	CHECK(NULL != passGen);
	CHECK(NULL != genPass);

	hashFunc = getHashFunFromName(HASH_FUNC_NAME);
	CHECK(NULL != hashFunc);

	allPasswordEnumeratorInitialize(&passEnumerator, passGen, passBuf);

	while(passwordEnumeratorCalculateNextPassword((passwordEnumerator_t *) &passEnumerator)) {
		CHECK(testCrackPassword(rt, hashFunc, passBuf));
/*		testCrackPassword(rt, hashFunc, passBuf);  */

	}
	
	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;


LBL_CLEANUP:
	return ret;

}

int main(int agrc, char ** argv) {
	int ret = -2;

	dictionary_t dictionary;
	passwordGenerator_t passwordGenerator;
	randomPasswordEnumerator_t randomPasswordEnumerator;
	char * generatorBuf = NULL;
	char * enumeratorBuf = NULL;

	RainbowTable_t * rt = NULL;
	

	(void) removeFiles();

	dictionaryInitialize(&dictionary, "hello\nworld");
	passwordGeneratorInitialize(&passwordGenerator, "||@&1-1|&1-1@", &dictionary);
	generatorBuf = (char *) malloc (passwordGeneratorGetMaxLength(&passwordGenerator) + 1);
	enumeratorBuf = (char *) malloc (passwordGeneratorGetMaxLength(&passwordGenerator) + 1);
	randomPasswordEnumeratorInitialize(&randomPasswordEnumerator,  &passwordGenerator, enumeratorBuf, 10 *passwordGeneratorGetSize(&passwordGenerator));
	
	/*while (passwordEnumeratorCalculateNextPassword((passwordEnumerator_t *) &randomPasswordEnumerator)) {
		printf("password=%s\n", buf);
	}*/

	CHECK(RT_generate((passwordEnumerator_t *) &randomPasswordEnumerator, &passwordGenerator,
				    enumeratorBuf, generatorBuf,
				    getHashFunFromName(HASH_FUNC_NAME),
				    RAINBOW_CHAIN_LEN,
				    HASH_TABLE_FILE_PREFIX,
				    HASH_TABLE_ENTRIES,
				    HASH_TABLE_PAIRS_PER_BLOCK));

	printf(">>> passed RT_generate\n");

	/* Now open the generated table */
	rt = RT_open(&passwordGenerator, generatorBuf,
		     HASH_TABLE_FILE_PREFIX);
	CHECK(NULL != rt);


	CHECK(testCrackAllPasswordsInRange(rt, &passwordGenerator, generatorBuf));
	printf(">>> passed testCrackAllPasswordsInRange\n");
	


	printf(">>> All tests passed\n");
	ret = 0;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = -1;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	return ret;

}
