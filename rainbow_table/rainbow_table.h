#ifndef __RAINBOW_TABLE_H__
#define __RAINBOW_TABLE_H__

#include "../common/types.h"
#include "../common/misc.h"
#include "../password/password_enumerator.h"
#include "../password/password_generator.h"
#include "../DEHT/DEHT.h"

typedef LONG_INDEX_PROJ RainbowSeed_t;

/* data stored inside DEHT's user bytes */
typedef struct RainbowTableConfig_s {
	ulong_t chainLength;
	RainbowSeed_t seeds[1];
} RainbowTableConfig_t;

/* instance */
typedef struct RainbowTable_s {
	DEHT * hashTable;
	RainbowTableConfig_t * config;
	BasicHashFunctionPtr hashFunc;
	const passwordGenerator_t * passGenerator;
	char * password;
	ulong_t passwordLength;
} RainbowTable_t;

/* only for generation - for queries, use the RT_open, RT_query interface */
bool_t RT_generate(passwordEnumerator_t * passwordEnumerator,
		   const passwordGenerator_t * passwordGenerator,
		   char * enumeratorPassword,
		   char * generatorPassword,
		   BasicHashFunctionPtr hashFunc,
		   ulong_t rainbowChainLen,
		   const char * hashTableFilePrefix,
		   ulong_t nHashTableEntries,
		   ulong_t nPairsPerBlock,
		   bool_t b1, /*! TODO: tmp */
		   bool_t b2 /*! TODO: tmp */);

RainbowTable_t * RT_open(const passwordGenerator_t * passGenerator,
			 char * password,
			 ulong_t passwordLength,
			 const char * hashTableFilePrefix,
			 bool_t enableFirstBlockCache,
			 bool_t enableLastBlockCache);

void RT_close(RainbowTable_t * self);

bool_t RT_query(RainbowTable_t * self, const byte_t * hash, ulong_t hashLen, bool_t * found);


bool_t RT_print(FILE * seedsAndPasswordsFd,
		FILE * chainsFd,

		const passwordGenerator_t * passGenerator,
		char * generatorPassword,
		ulong_t passwordMaxLen,
		const char * hashTableFilePrefix);

/*
void goDownChain(byte_t * curHash,
		uint_t curHashLen,
		const passwordGenerator_t * passwordGenerator,
		ulong_t numPossiblePasswords,
		char * pass,
		BasicHashFunctionPtr cryptHashPtr,
		const RainbowSeed_t * seeds,
		ulong_t iterations);
*/

#endif /* __RAINBOW_TABLE_H__ */
