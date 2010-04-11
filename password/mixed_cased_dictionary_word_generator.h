#ifndef __MIXED_CASED_DICTIONARY_WORD_GENERATOR_H__
#define __MIXED_CASED_DICTIONARY_WORD_GENERATOR_H__

#include "../common/types.h"
#include "dictionary.h"
#include "dictionary_word_generator.h"

/**
 * A variation of dictionary word generator which generates words with mixed case.
 *
 * The number of casing possibilities for each word is pre-computed during the
 * initialization of the mixed cased dictionary word generator, s.t. given an
 * index, finding the word the case of which should be mixed costs only
 * O(log(#{dictionary entries})), rather than O(#{dictionary entries}). That is
 * achieved using binary search.
 */
typedef struct mixedCasedDictionaryWordGenerator_s {
	dictionaryWordGenerator_t super;
	
	/* The i'th element is the first index which corresponds with a case
	 * mixing of the i'th dictionary entry. The last element is the first
	 * index after the range */
	ulong_t * beginIndexes;
} mixedCasedDictionaryWordGenerator_t;

/**
 * @see dictionaryWordGeneratorInitialize
 *
 * @return FALSE iff failed allocating memory
 */
bool_t mixedCasedDictionaryWordGeneratorInitialize(mixedCasedDictionaryWordGenerator_t * self,
						   const dictionary_t * dictionary);
/**
 * Finalization function
 * Frees the memory used by the dictionary word generator
 *
 * @param self		An dictionary word generator
 *
 * @pre		self != NULL
 * @pre		self is initialized
 */
void mixedCasedDictionaryWordGeneratorFinalize(mixedCasedDictionaryWordGenerator_t * self);

/**
 * @see	passwordPartGenerator_t
 */
char * mixedCasedDictionaryWordGeneratorCalcPass(const mixedCasedDictionaryWordGenerator_t * self,
						 ulong_t index,
						 char * buf);

#endif /*__MIXED_CASED_DICTIONARY_WORD_GENERATOR_H__*/
