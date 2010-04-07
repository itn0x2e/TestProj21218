#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rand_utils.h"

byte_t getRandomByte() {
	static bool_t initialized = FALSE;
	
	if (!initialized) {
		/* initialize random seed */
		srand (time(NULL));
		initialized = TRUE;
	}
	
	return (byte_t) rand();
}

ulong_t getRandomULong() {
	ulong_t randNum;
	randomizeBuffer((byte_t *) &randNum, sizeof(randNum));
	return randNum;
}

void randomizeBuffer(byte_t * buf, uint_t len) {
	uint_t i;
	
	for (i = 0; i < len; ++i) {
		buf[i] = getRandomByte();
	}
}

void randomizeSalt(byte_t * salt) {
	randomizeBuffer(salt, SALT_LEN);
}
