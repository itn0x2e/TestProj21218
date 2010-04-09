#include "my_rainbow_table.h"

static DEHT * createEmptyRainbowTable();
static bool_t fillRainbowTable(passwordEnumerator_t * passwordEnumerator,
			       char * firstPass,
			       BasicHashFunctionPtr cryptHashPtr,
			       ulong_t chainLength,
			       LONG_INDEX_PROJ * seeds);
static bool_t insertIntoDEHT(const byte_t * key, uint_t keyLen, const char * dataStr);

bool_t RT_generate(passwordEnumerator_t * passwordEnumerator,
		   const passwordGenerator_t * passwordGenerator,
		   char * enumeratorPassword,
		   char * generatorPassword,
		   BasicHashFunctionPtr hashFunc,
		   ulong_t rainbowChainLen,
		   const char * hashTableFilePrefix,
		   ulong_t nHashTableEntries,
		   ulong_t nPairsPerBlock) {
	char * firstPass = generatorPassword;
	byte_t curHash[MAX_DIGEST_LEN];
	
	createEmptyRainbowTable();
	
	fillRainbowTable(passwordEnumerator, passwordGenerator, enumeratorPassword, generatorPassword, hashFunc, rainbowChainLen,
			       LONG_INDEX_PROJ * seeds)
		
}

RainbowTable_t * RT_open(const passwordGenerator_t * passGenerator,
			 char * generatorPassword,
			 const char * hashTableFilePrefix,
			 bool_t enableFirstBlockCache,
			 bool_t enableLastBlockCache);

void RT_close(RainbowTable_t * self);

bool_t RT_query(RainbowTable_t * self, byte_t * hash, ulong_t hashLen, char * resPassword, ulong_t resPasswordLen);

static DEHT * createEmptyRainbowTable() {
	/*!TODO: dummy*/
}

static bool_t fillRainbowTable(passwordEnumerator_t * passwordEnumerator,
			       const passwordGenerator_t * passwordGenerator,
			       char * firstPass,
			       char * kthPass,
			       BasicHashFunctionPtr cryptHashPtr,
			       ulong_t chainLength,
			       LONG_INDEX_PROJ * seeds) {
	/* Iterate many times (about 10 times size of S) */
	while(passwordEnumeratorCalculateNextPassword()) {
		/* Init curHash := cryptographic-hash(firstPass) */
		byte_t curHash[MAX_DIGEST_LEN];
		uint_t curHashLen = (uint_t) cryptHash(cryptHashPtr, firstPass, curHash);
		/* For j=1 to chain-length do */
		ulong_t j;
		for (j = 0; j < chainLength; ++j) {
			/* k = pseudo-random-function with seed seed[j] and input curHash; */
			LONG_INDEX_PROJ k = pseudo_random_function(curHash, curHashLen, seed[j]);
			passwordGeneratorCalculatePassword(passwordGenerator, k, kthPass);
			/* curHash = cryptographic-hash(get_kth_password_64b(k,S)); */
			cryptHash(cryptHashPtr, kthPass, curHash);
		/* end */
		}
		/* insert into disk embedded hash table the following pair: key=curHash, data=firstPass */
		if (!insertIntoDEHT(curHash, curHashLen, firstPass)) {
			return FALSE;
		}
	/* end */
	}
	return TRUE;
}
