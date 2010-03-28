#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include "../common/types.h"

typedef struct dictionary_s {
	const char ** entries,
	unsigned int numEntries
} dictionary_t;

bool_t dictionaryInitialize(dictionary_t * dictionary,
			    const char * filename);
bool_t dictionaryFinalize();
unsigned long dictionaryGetSize();
const char * dictionaryGetEntry(unsigned long index);

#endif /*DICTIONARY_H_*/