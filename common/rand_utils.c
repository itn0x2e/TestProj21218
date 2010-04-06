#include <stdlib.h>
#include <time.h>
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
