#include "../common/utils.h"
#include "password_enumerator.h"

void passwordEnumeratorInitialize(passwordEnumerator_t * self, const passwordGenerator_t * generator, char * buf) {
	self->generator = generator;
	self->buf = buf;
	self->size = passwordGeneratorGetSize(self->generator);
	self->index = 0;
	self->getGeneratorIndexFunc = NULL;
}

bool_t passwordEnumeratorCalculateNextPassword(passwordEnumerator_t* self) {
	ASSERT(NULL != self->getGeneratorIndexFunc);

	if (self->index >= self->size) {
		return FALSE;
	}

	/* TODO: document self->index++ */
	if (!passwordGeneratorCalculatePassword(self->generator, self->getGeneratorIndexFunc(self), self->buf)) {
		return FALSE;
	}

	self->index++;

	return TRUE;
}
