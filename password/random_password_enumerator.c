#include "../common/rand_utils.h"
#include "../common/utils.h"
#include "random_password_enumerator.h"

static ulong_t getRandomIndex(const passwordEnumerator_t * passwordEnumerator);

void randomPasswordEnumeratorInitialize(randomPasswordEnumerator_t * self, const passwordGenerator_t * generator, char * buf, ulong_t iterations) {
	passwordEnumerator_t * enumerator =  (passwordEnumerator_t *) self;

	passwordEnumeratorInitialize(enumerator, generator, buf);

	enumerator->size = iterations;
	enumerator->getGeneratorIndexFunc = getRandomIndex;
}

static ulong_t getRandomIndex(const passwordEnumerator_t * passwordEnumerator) {
	/* TODO: document that there is a precondition that this function is invoked only if (passwordEnumerator->size > 0) */
	ASSERT(passwordEnumerator->size > 0);

	return (getRandomULong() % passwordEnumerator->size);
}
