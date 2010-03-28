#ifndef PASSWORD_GENERATOR_H_
#define PASSWORD_GENERATOR_H_

#include "../common/types.h"
#include "dictionary.h"

typedef struct passwordGenerator_s {
	const char ** ruleEntries,
	unsigned int numRuleEntries,
	dictionary_t dictionary,
	unsigned long maxPasswordLength
} passwordGenerator_t;

bool_t passwordGeneratorInitialize(passwordGenerator_t * passwordGenerator,
				   const char * rule,
				   dictionary_t * dictionary);
bool_t passwordGeneratorFinalize();
unsigned long passwordGeneratorGetSize();
const char * passwordGeneratorCalculatePassword(unsigned long index);
unsigned long passwordGeneratorGetRuleEntrySize(const char * ruleEntry);
unsigned long passwordGeneratorGetRuleEntryMaxLength(const char * ruleEntry);
unsigned long passwordGeneratorGetDictionaryMaxLength();

#endif /*PASSWORD_GENERATOR_H_*/