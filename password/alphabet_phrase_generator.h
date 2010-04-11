#ifndef __ALPHABET_PHRASE_GENERATOR_H__
#define __ALPHABET_PHRASE_GENERATOR_H__

#include "../common/types.h"
#include "password_part_generator.h"

/**
 * Specifies the alphabet of which passwords are generated
 */
typedef enum alphabet_e {
	ALPHABET_DIGIT,
	ALPHABET_ALPHA,
	ALPHABET_ALNUM,
	ALPHABET_PRINTABLE
} alphabet_t;

/**
 * A variation of a password part generator which generates phrases consisting
 * of symbols os a specified alphabet. The length of each phrase is within a
 * specified range.
 */
typedef struct alphabetPhraseGenerator_s {
	passwordPartGenerator_t super;
	
	/* The symbols available in the specified alphabet */
	const char * alphabetChars;
	
	/* The number of symbols in the specified alphabet */
	uint_t alphabetLength;
	
	/* The index of the first phrase in the specified range, relatively to
	 * the length range which begins with 0 */
	ulong_t startIndex;
} alphabetPhraseGenerator_t;

/**
 * Initialization function
 *
 * @param self		The alphabet phrase generator to initialize
 * @param minLength	The minimal length
 * @param maxLength	The maximal length
 *
 * @pre		self != NULL
 * @pre		minLength <= maxLength
 */
void alphabetPhraseGeneratorInitialize(alphabetPhraseGenerator_t * self,
				       alphabet_t alphabet, uint_t minLength,
				       uint_t maxLength);
/**
 * Finalization function
 *
 * @param self		An alphabet phrase generator
 *
 * @pre		self != NULL
 * @pre		self is initialized
 */
void alphabetPhraseGeneratorFinalize(alphabetPhraseGenerator_t * self);

/**
 * @see	passwordPartGenerator_t
 */
char * alphabetPhraseGeneratorCalcPass(const alphabetPhraseGenerator_t * self,
				       ulong_t index,
				       char * buf);

#endif /* __ALPHABET_PHRASE_GENERATOR_H__ */
