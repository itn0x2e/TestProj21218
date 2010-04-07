#ifndef __DICTIONARY_WORD_GENERATOR_H__
#define __DICTIONARY_WORD_GENERATOR_H__

#include "../common/types.h"
#include "dictionary.h"
#include "password_part_generator.h"

typedef struct dictionaryWordGenerator_s {
	passwordPartGenerator_t super;
	const dictionary_t * dictionary;
} dictionaryWordGenerator_t;

void dictionaryWordGeneratorInitialize(dictionaryWordGenerator_t * self, const dictionary_t * dictionary);

#endif /* __DICTIONARY_WORD_GENERATOR_H__ */