#ifndef __DICTIONARY_WORD_GENERATOR_H__
#define __DICTIONARY_WORD_GENERATOR_H__

#include "../common/types.h"
#include "dictionary.h"
#include "password_part_generator.h"

/**
 * A variation of a password part generator which generates dictionary words
 */
typedef struct dictionaryWordGenerator_s {
	passwordPartGenerator_t super;
	
	/* The wrapped dictionary */
	const dictionary_t * dictionary;
} dictionaryWordGenerator_t;

/**
 * Initialization function
 *
 * @param self		The dictionary word generator to initialize
 * @param dictionary	The wrapped dictionary
 *
 * @pre		self != NULL
 * @pre		dictionary != NULL
 * @pre		dictionary is initialized
 */
void dictionaryWordGeneratorInitialize(dictionaryWordGenerator_t * self,
				       const dictionary_t * dictionary);

#endif /* __DICTIONARY_WORD_GENERATOR_H__ */
