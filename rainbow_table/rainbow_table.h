
#ifndef __RAINBOW_TABLE_H__
#define __RAINBOW_TABLE_H__

#include "../common/types.h"

#include "../password/password_enumerator.h"

bool_t createRainbowTable(
			passwordEnumerator_t * passEnumerator,
			char * generatedPassword,

			BasicHashFunctionPtr hashFunc,

			ulong_t rainbowChainLen,

			char * hashTableFilePrefix,
			ulong_t nHashTableEntries,
			ulong_t nPairsPerBlock,
			bool_t enableFirstBlockCache,
			bool_t enableLastBlockCache);

#endif /* __RAINBOW_TABLE_H__ */
