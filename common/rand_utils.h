#ifndef __RAND_UTILS_H__
#define __RAND_UTILS_H__

#include "types.h"

ulong_t getRandomULong();

void randomizeSalt(byte_t * salt); /* TODO: document that salt is assumed to have at least 6 allocated bytes */

#endif /* __RAND_UTILS_H__ */
