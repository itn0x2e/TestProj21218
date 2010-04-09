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
	byte_t * generatorPassword;

} RainbowTable_t;




/* only for generation - for queries, use the RT_open, RT_query interface */
bool_t RT_generate(	passwordEnumerator_t * passEnumerator,
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

RainbowTable_t * RT_open(
			const passwordGenerator_t * passGenerator,
			char * generatorPassword,

			const char * hashTableFilePrefix,
			bool_t enableFirstBlockCache,
			bool_t enableLastBlockCache);

void RT_close(RainbowTable_t * self);

bool_t RT_query(RainbowTable_t * self, 
		byte_t * hash, ulong_t hashLen,
		char * resPassword, ulong_t resPasswordLen);


#endif /* __RAINBOW_TABLE_H__ */
