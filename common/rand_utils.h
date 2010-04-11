#ifndef __RAND_UTILS_H__
#define __RAND_UTILS_H__

#include "misc.h"
#include "types.h"

#define SALT_LEN (6)

byte_t getRandomByte();
ulong_t getRandomULong();
LONG_INDEX_PROJ getRandomLongIndexProj();
void randomizeBuffer(byte_t * buf, uint_t len);
void randomizeSalt(byte_t * salt); /* salt is assumed to have at least 6 allocated bytes */

#endif /* __RAND_UTILS_H__ */
