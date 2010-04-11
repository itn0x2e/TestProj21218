#ifndef __SINGLE_CASED_DICTIONARY_WORD_GENERATOR_H__
#define __SINGLE_CASED_DICTIONARY_WORD_GENERATOR_H__

#include "../common/types.h"
#include "dictionary.h"
#include "dictionary_word_generator.h"

/**
 * A variation of dictionary word generator which generates words with
 * a specified case.
 */
typedef struct singleCasedDictionaryWordGenerator_s {
	dictionaryWordGenerator_t super;
	
	/* The letter case */
	letterCase_t letterCase;
} singleCasedDictionaryWordGenerator_t;

/**
 * @see dictionaryWordGeneratorInitialize
 *
 * @param letterCase 	The letter case to use
 */
void singleCasedDictionaryWordGeneratorInitialize(singleCasedDictionaryWordGenerator_t * self,
						  const dictionary_t * dictionary,
						  letterCase_t letterCase);
						  
/**
 * Finalization function
 */
void singleCasedDictionaryWordGeneratorFinalize(singleCasedDictionaryWordGenerator_t * self);

/**
 * @see	passwordPartGenerator_t
 */
char * singleCasedDictionaryWordGeneratorCalcPass(const singleCasedDictionaryWordGenerator_t * self,
						  ulong_t index,
						  char * buf);

#endif /*__SINGLE_CASED_DICTIONARY_WORD_GENERATOR_H__*/
