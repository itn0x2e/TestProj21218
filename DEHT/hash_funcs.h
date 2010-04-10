#ifndef __DEHT_HASH_FUNCS_H__
#define __DEHT_HASH_FUNCS_H__


/* 64 / 8 */
#define BYTES_PER_KEY (8)
#define TABLE_INDEX_SEED ("Chunk norris counted to infinity")
#define VALIDATION_SEED ("Twice!")


/**
* Function brief description: A function that can be used as the key-to-
*			      hash table-index function in DEHT
* Function desc: This function takes a key of keySize bytes,
*		 and returns an index between 0 and tableSize (not incl.),
*		 for use as a bucket selection hash in DEHT.
*		 Internally, this is done by hashing the key and a hard-coded seed,
* 		 and taking a subset of the resulting hash and wrapping it around tableSize.
*
* @param key - data to hash
* @param keySize - number of bytes in the data buffer
* @param tableSize - number of buckets in hash table
*
* @ret An index between 0 and tableSize
*      on subsequent calls, DEHT_STATUS_FAIL on failure.
*
*/
int DEHT_keyToTableIndexHasher(const unsigned char * key, int keySize, int tableSize);


/**
* Function brief description: A function that can be used as the validation key
*			      calculation function in DEHT.
* Function desc: This function takes a key of keySize bytes,
*		 and returns 8 bytes of output (64 bits) in resBuf.
*		 Internally, this is done by hashing the key and a different hard-coded seed,
* 		 and taking a subset of the resulting hash.
*
* @param key - data to hash
* @param keySize - number of bytes in the data buffer
* @param resBuf - a buffer that will receive the hash output
*
* @ret always 0 (the project spec didn't specify anything as to these function's return value)
*
*/
int DEHT_keyToValidationKeyHasher64(const unsigned char * key,int keySize, unsigned char * resBuf);

/**
* Function brief description: A function that can be used as the validation key
*			      calculation function in DEHT.
* Function desc: This function takes a key of keySize bytes,
*		 and returns 16 bytes of output (128 bits) in resBuf.
*		 Internally, this is done by hashing the key and a different hard-coded seed,
* 		 and taking a subset of the resulting hash.
*
* @param key - data to hash
* @param keySize - number of bytes in the data buffer
* @param resBuf - a buffer that will receive the hash output
*
* @ret always 0 (the project spec didn't specify anything as to these function's return value)
*
*/
int DEHT_keyToValidationKeyHasher128(const unsigned char * key,int keySize, unsigned char * resBuf);

#endif /* __DEHT_HASH_FUNCS_H__ */
