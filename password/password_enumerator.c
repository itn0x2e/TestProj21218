#include "../common/utils.h"
#include "password_enumerator.h"

void passwordEnumeratorInitialize(passwordEnumerator_t * self,
								  const passwordGenerator_t * generator,
								  char * password,
								  ulong_t iterations,
								  getGeneratorIndexFuncPtr_t getGeneratorIndexFunc) {
	self->generator = generator;
	self->password = password;
	self->iterations = iterations;
	self->index = 0;
	self->getGeneratorIndexFunc = NULL;
}

bool_t passwordEnumeratorCalculateNextPassword(passwordEnumerator_t* self) {
	ASSERT(NULL != self->getGeneratorIndexFunc);

	if (self->index >= self->iterations) {
		return FALSE;
	}

	passwordGeneratorCalculatePassword(self->generator,
									   self->getGeneratorIndexFunc(self),
									   self->password);
	self->index++;
	return TRUE;
}
