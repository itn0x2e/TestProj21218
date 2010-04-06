#ifndef __MIXED_CASED_DICTIONARY_WORD_GENERATOR_H__
#define __MIXED_CASED_DICTIONARY_WORD_GENERATOR_H__

#include "../common/types.h"
#include "dictionary.h"
#include "dictionary_word_generator.h"

typedef struct mixedCasedDictionaryWordGenerator_s {
	dictionaryWordGenerator_t super;
	ulong_t * beginIndexes;
} mixedCasedDictionaryWordGenerator_t;

bool_t mixedCasedDictionaryWordGeneratorInitialize(mixedCasedDictionaryWordGenerator_t * self, const dictionary_t * dictionary);
void mixedCasedDictionaryWordGeneratorFinalize(mixedCasedDictionaryWordGenerator_t * self);
char * mixedCasedDictionaryWordGeneratorCalcPass(const mixedCasedDictionaryWordGenerator_t * self, ulong_t index, char * buf);

#endif /*__MIXED_CASED_DICTIONARY_WORD_GENERATOR_H__*/

