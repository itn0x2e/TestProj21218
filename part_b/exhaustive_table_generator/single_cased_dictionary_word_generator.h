#ifndef __SINGLE_CASED_DICTIONARY_WORD_GENERATOR_H__
#define __SINGLE_CASED_DICTIONARY_WORD_GENERATOR_H__

#include "../../common/types.h"
#include "dictionary.h"
#include "dictionary_word_generator.h"

typedef struct singleCasedDictionaryWordGenerator_s {
	dictionaryWordGenerator_t super;
	letterCase_t letterCase;
} singleCasedDictionaryWordGenerator_t;

inline void singleCasedDictionaryWordGeneratorInitialize(singleCasedDictionaryWordGenerator_t * self, const dictionary_t * dictionary, letterCase_t letterCase);
inline void singleCasedDictionaryWordGeneratorFinalize(singleCasedDictionaryWordGenerator_t * self);
char * singleCasedDictionaryWordGeneratorCalcPass(const singleCasedDictionaryWordGenerator_t * self, ulong_t index, char * buf);

#endif /*__SINGLE_CASED_DICTIONARY_WORD_GENERATOR_H__*/
