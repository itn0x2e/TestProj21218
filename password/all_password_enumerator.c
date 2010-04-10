#include "all_password_enumerator.h"

/**
 * TODO
 */
static ulong_t getEnumeratorIndex(const passwordEnumerator_t * passwordEnumerator);

void allPasswordEnumeratorInitialize(allPasswordEnumerator_t * self,
									 const passwordGenerator_t * generator,
									 char * password) {
	passwordEnumeratorInitialize((passwordEnumerator_t *) self,
								 generator,
								 password,
								 passwordGeneratorGetSize(generator),
								 getEnumeratorIndex);
}

static ulong_t getEnumeratorIndex(const passwordEnumerator_t * passwordEnumerator) {
	return passwordEnumerator->index;
}
