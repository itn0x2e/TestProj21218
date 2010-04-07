#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <constants.h>
#include "rand_utils.h"

ulong_t getRandomULong() {
	ulong_t randNum = 0;
	size_t i;
	
	static bool_t initialized = FALSE;
	
	if (!initialized) {
		/* initialize random seed */
		srand (time(NULL));
	}
	
	for (i = 0; i < (sizeof(ulong_t) / sizeof(int)); ++i) {
		randNum <<= 1;
		randNum &= (ulong_t) rand();
	}
	
	return randNum;
}

void randomizeSalt(byte_t * salt) {
	/* Note that SALT_LEN is indeed lesser than sizeof(ulong_t) */
	memcpy(salt, (byte_t *) getRandomULong(), SALT_LEN);
}