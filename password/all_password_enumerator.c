#include "all_password_enumerator.h"

static ulong_t getEnumeratorIndex(const passwordEnumerator_t * passwordEnumerator);

void allPasswordEnumeratorInitialize(allPasswordEnumerator_t * self, const passwordGenerator_t * generator, char * buf) {
	passwordEnumerator_t * enumerator =  (passwordEnumerator_t *) self;

	passwordEnumeratorInitialize(enumerator, generator, buf);

	enumerator->size = passwordGeneratorGetSize(generator);
	enumerator->getGeneratorIndexFunc = getEnumeratorIndex;
}

static ulong_t getEnumeratorIndex(const passwordEnumerator_t * passwordEnumerator) {
	return passwordEnumerator->index;
}
