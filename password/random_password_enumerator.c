#include "../common/rand_utils.h"
#include "../common/utils.h"
#include "random_password_enumerator.h"

/**
 * TODO
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
	/* TODO: document that there is a precondition that this function is invoked only if (generatorSize > 0) */
	return (getRandomULong() % ((randomPasswordEnumerator_t *) passwordEnumerator)->generatorSize);
}
