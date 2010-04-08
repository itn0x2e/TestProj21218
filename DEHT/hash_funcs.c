
#include "hash_funcs.h"
#include "../common/types.h"
#include "../common/misc.h"



int DEHT_keyToTableIndexHasher(const unsigned char * key, int keySize, int tableSize)
{
	int cksum = 0;

	TRACE_FUNC_ENTRY();

	/* ignoring errors, since miniHash is very very unlikely to fail */
	(void) miniHash((byte_t *) &cksum, sizeof(cksum), (byte_t *) TABLE_INDEX_SEED, sizeof(TABLE_INDEX_SEED) - 1, key, keySize);

	/* ignore sign bit */
	cksum = abs(cksum);

	TRACE_FUNC_EXIT();
	return (cksum % tableSize);
}

int DEHT_keyToValidationKeyHasher64(const unsigned char * key,int keySize, unsigned char * resBuf)
{
	TRACE_FUNC_ENTRY();

	/* ignoring errors, since miniHash is very very unlikely to fail */
	(void) miniHash(resBuf, 8, (byte_t *) VALIDATION_SEED, sizeof(VALIDATION_SEED) - 1, key, keySize);

	TRACE_FUNC_EXIT();
	return 0;
}

int DEHT_keyToValidationKeyHasher128(const unsigned char * key,int keySize, unsigned char * resBuf)
{
	TRACE_FUNC_ENTRY();

	/* ignoring errors, since miniHash is very very unlikely to fail */
	(void) miniHash(resBuf, 16, (byte_t *) VALIDATION_SEED, sizeof(VALIDATION_SEED) - 1, key, keySize);

	TRACE_FUNC_EXIT();
	return 0;
}

