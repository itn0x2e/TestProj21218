#ifndef __PASSWORD_ENUMERATOR_H__
#define __PASSWORD_ENUMERATOR_H__

#include "../common/types.h"
#include "password_generator.h"

struct passwordEnumerator_s;

typedef ulong_t (* getGeneratorIndexFuncPtr_t) (const struct passwordEnumerator_s * self);

typedef struct passwordEnumerator_s {
	const passwordGenerator_t * generator;
	char * buf;
	ulong_t size;
	ulong_t index;
	getGeneratorIndexFuncPtr_t getGeneratorIndexFunc;
} passwordEnumerator_t;

void passwordEnumeratorInitialize(passwordEnumerator_t * self, const passwordGenerator_t * generator, char * buf);
bool_t passwordEnumeratorCalculateNextPassword(passwordEnumerator_t* self);

#endif /* __PASSWORD_ENUMERATOR_H__ */
