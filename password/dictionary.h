#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include "../common/types.h"

typedef enum letterCast_e {
	LOWER_CASE = 0,
	UPPER_CASE = 1
} letterCase_t;

typedef char * stringPair_t[2];

typedef struct dictionary_s {
	uint_t numEntries;
	uint_t maxEntryLength;
	char * rawDataLowerUpper; /* a copy of the raw data - lower cased followed by upper cased */
	stringPair_t * entries;
} dictionary_t;

bool_t dictionaryInitialize(dictionary_t * self, const char * rawDict);
void dictionaryFinalize(dictionary_t * self);
ulong_t dictionaryGetSize(const dictionary_t * self);
ulong_t dictionaryGetMaxLength(const dictionary_t * self);
const char * dictionaryGetEntry(const dictionary_t * self, ulong_t index, letterCase_t letterCase);

#endif /* __DICTIONARY_H__ */
