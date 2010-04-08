#ifndef __RAINBOW_TABLE_H__
#define __RAINBOW_TABLE_H__

#include "../common/types.h"
#include "../common/misc.h"
#include "../password/password_enumerator.h"
#include "../password/password_generator.h"
#include "../DEHT/DEHT.h"


typedef ulong_t RainbowSeed_t;

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

} RainbowTable_t;


/* only for generation */
bool_t generateRainbowTable(
			passwordEnumerator_t * passEnumerator,
			const passwordGenerator_t * passGenerator,

			char * enumeratorPassword,
			char * generatorPassword,

			BasicHashFunctionPtr hashFunc,

			ulong_t rainbowChainLen,

			const char * hashTableFilePrefix,
			ulong_t nHashTableEntries,
			ulong_t nPairsPerBlock,
			bool_t enableFirstBlockCache,
			bool_t enableLastBlockCache);

bool_t query(byte_t * hash, ulong_t hashLen,
			    byte_t * password, ulong_t passwordLen);

bool_t openRainbowTable(RainbowTable_t * instance,
			const passwordGenerator_t * passGenerator,
			char * generatorPassword,

			BasicHashFunctionPtr hashFunc,

			ulong_t rainbowChainLen,

			const char * hashTableFilePrefix,
			bool_t enableFirstBlockCache,
			bool_t enableLastBlockCache);

bool_t closeRainbowTable(RainbowTable_t * instance);

#endif /* __RAINBOW_TABLE_H__ */
