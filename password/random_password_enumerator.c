#include "../common/rand_utils.h"
#include "../common/utils.h"
#include "random_password_enumerator.h"

/**
 * @param passwordEnumerator - a password enumerator
 * @pre		NULL != passwordEnumerator
 * @pre		passwordEnumerator is initialized as a random password enumerator
 * @pre		(randomPasswordEnumerator_t *) passwordEnumerator)->generatorSize) > 0
 *
 * @return	A pseudo-random ulong_t lesser than generatorSize
 */
static ulong_t getRandomIndex(const passwordEnumerator_t * passwordEnumerator);

void randomPasswordEnumeratorInitialize(randomPasswordEnumerator_t * self,
					const passwordGenerator_t * generator,
					char * password,
					ulong_t iterations) {
	passwordEnumeratorInitialize((passwordEnumerator_t *) self,
				     generator,
				     password,
				     iterations,
				     getRandomIndex);
	self->generatorSize = passwordGeneratorGetSize(generator);
}

static ulong_t getRandomIndex(const passwordEnumerator_t * passwordEnumerator) {
	return (getRandomULong() % ((randomPasswordEnumerator_t *) passwordEnumerator)->generatorSize);
}
