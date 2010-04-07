#ifndef __RAND_UTILS_H__
#define __RAND_UTILS_H__

#include "types.h"

#define SALT_LEN (6)

byte_t getRandomByte();
ulong_t getRandomULong();
void randomizeBuffer(byte_t * buf, uint_t len);
void randomizeSalt(byte_t * salt); /* TODO: document that salt is assumed to have at least 6 allocated bytes */

#endif /* __RAND_UTILS_H__ */
