#ifndef __DICTIONARY_WORD_GENERATOR_H__
#define __DICTIONARY_WORD_GENERATOR_H__

#include "../../common/types.h"
#include "dictionary.h"
#include "password_part_generator.h"

typedef struct dictionaryWordGenerator_s {
	passwordPartGenerator_t super;
	dictionary_t * dictionary;
} dictionaryWordGenerator_t;

inline void dictionaryWordGeneratorInitialize(dictionaryWordGenerator_t * self, dictionary_t * dictionary);

#endif /*__DICTIONARY_WORD_GENERATOR_H__*/
