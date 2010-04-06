#ifndef __ALL_PASSWORD_ENUMERATOR_H__
#define __ALL_PASSWORD_ENUMERATOR_H__

#include "password_enumerator.h"

typedef struct allPasswordEnumerator_s {
	passwordEnumerator_t super;
} allPasswordEnumerator_t;

void allPasswordEnumeratorInitialize(allPasswordEnumerator_t * self, const passwordGenerator_t * generator, char * buf);

#endif /* __ALL_PASSWORD_ENUMERATOR_H__ */
