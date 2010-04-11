#ifndef __ALL_PASSWORD_ENUMERATOR_H__
#define __ALL_PASSWORD_ENUMERATOR_H__

#include "password_enumerator.h"

/**
 * A variation of password enumerator which enumerates all passwords available
 * by the password generator.
 */
typedef struct allPasswordEnumerator_s {
	passwordEnumerator_t super;
} allPasswordEnumerator_t;

/**
 * An initialization function.
 *
 * @param self		The all password enumerator to initialize
 * @param generator	The wrapped password generator
 * @param password	A buffer into which passwords should be written
 *
 * @pre		self != NULL
 * @pre		generator != NULL
 * @pre		The wrapped password generator is valid and initialized.
 * @pre		The buffer supplied is large enough to contain, including a
 * 		terminating null, any password generated by the generator.
 */
void allPasswordEnumeratorInitialize(allPasswordEnumerator_t * self,
				     const passwordGenerator_t * generator,
				     char * password);

#endif /* __ALL_PASSWORD_ENUMERATOR_H__ */
