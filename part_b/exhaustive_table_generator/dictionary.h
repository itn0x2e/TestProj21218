#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include "../../common/types.h"

typedef enum letterCast_e {
	LOWER_CASE = 0,
	UPPER_CASE = 1
} letterCase_t;

typedef struct dictionary_s {
	const char ** entries;
	uint_t numEntries;
} dictionary_t;

bool_t dictionaryInitialize(dictionary_t * self, const char * filename);
void dictionaryFinalize(dictionary_t * self);
inline ulong_t dictionaryGetSize(const dictionary_t * self);
inline ulong_t dictionaryGetMaxLength(const dictionary_t * self);
inline const char * dictionaryGetEntry(const dictionary_t * self, ulong_t index, letterCase_t letterCase);

#endif /*__DICTIONARY_H__*/
