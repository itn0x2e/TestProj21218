#ifndef __PASSWORD_PART_GENERATOR_H_
#define __PASSWORD_PART_GENERATOR_H_

#include "../common/types.h"
#include "dictionary.h"

struct passwordPartGenerator_s;

typedef char * (* passwordPartGeneratorCalcPassFuncPtr_t) (const struct passwordPartGenerator_s *, ulong_t, char *);
typedef void (* passwordPartGeneratorFinalizeFuncPtr_t) (struct passwordPartGenerator_s *);

/**
 * A generator of a rule part according to a single term of the rule
 */
typedef struct passwordPartGenerator_s {
	/* The total number of valid password part indexes */
	ulong_t size;
	
	/* The length of the longest password part, not including the terminating null */
	ulong_t maxLength;
	
	/* The function which determines the internal implementation of
	 * passwordPartGeneratorCalculatePassword */
	passwordPartGeneratorCalcPassFuncPtr_t calcPassFunc;
	
	/* The function which determines the internal implementation of
	 * passwordPartGeneratorFinalize */
	passwordPartGeneratorFinalizeFuncPtr_t finalizeFunc;
} passwordPartGenerator_t;

/**
 * Factory function
 *
 * @param rulePart	A string representation of the rule part
 * @param dictionary	The dictionary to use whenever specified by the rule part
 *
 * @pre		self != NULL
 * @pre		rule part has a valid syntax
 * @pre		dictionary != NULL
 * @pre		dictionary is initialized
 *
 * @return 	A pointer to a newly allocated password part corresponding to the rule
 *		part, or NULL if allocation fails.
 *
 * @note The rule part string does not have to be null-terminated.
 */
passwordPartGenerator_t * passwordPartGeneratorCreate(const char * rulePart,
						      const dictionary_t * dictionary);

/**
 * Finalization function.
 *
 * @param self	The password part generator to initialize
 * @pre		self != NULL
 */
void passwordPartGeneratorInitialize(passwordPartGenerator_t * self);

/**
 * Finalization function.
 *
 * @param self	The password part generator to finalize
 * @pre		self != NULL
 * @pre		self is initialized.
 */
void passwordPartGeneratorFinalize(passwordPartGenerator_t * self);

/**
 * @param self	A password part generator
 * @pre		self != NULL
 * @pre		self is initialized.
 *
 * @return	The total number of valid password part indexes
 */
ulong_t passwordPartGeneratorGetSize(const passwordPartGenerator_t * self);

/**
 * @param self	A password part generator
 * @pre		self != NULL
 * @pre		self is initialized.
 *
 * @return	The length of the longest password part, not including the
 *		terminating null
 */
ulong_t passwordPartGeneratorGetMaxLength(const passwordPartGenerator_t * self);

/**
 * Calculates the password part in a specified index and stores it into a specified
 * buffer.
 *
 * @param	self	A password part generator
 * @param	index	The index
 * @param	buf	A buffer into which to write the corresponding password
 *
 * @pre		self != NULL
 * @pre		self is initialized.
 * @pre		index < passwordPartGeneratorGetSize(self)
 * @pre		buf != NULL
 * @pre		buf is large enough to contain any password part generated by the
 *		generator.
 *
 * @return A pointer to the character which follows the last character written.
 */
char * passwordPartGeneratorCalculatePassword(const passwordPartGenerator_t * self, ulong_t index, char * buf);

#endif /*__PASSWORD_PART_GENERATOR_H_*/
