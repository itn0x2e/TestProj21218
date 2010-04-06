#ifndef __ALPHABET_PHRASE_GENERATOR_H__
#define __ALPHABET_PHRASE_GENERATOR_H__

#include "../common/types.h"
#include "password_part_generator.h"

typedef enum alphabet_e {
	ALPHABET_DIGIT,
	ALPHABET_ALPHA,
	ALPHABET_ALNUM,
	ALPHABET_PRINTABLE
} alphabet_t;

typedef struct alphabetPhraseGenerator_s {
	passwordPartGenerator_t super;
	const char * alphabetChars;
	uint_t alphabetLength;
	uint_t startIndex;
} alphabetPhraseGenerator_t;

void alphabetPhraseGeneratorInitialize(alphabetPhraseGenerator_t * self, alphabet_t alphabet, uint_t minLength, uint_t maxLength);
void alphabetPhraseGeneratorFinalize(alphabetPhraseGenerator_t * self);
char * alphabetPhraseGeneratorCalcPass(const alphabetPhraseGenerator_t * self, ulong_t index, char * buf);

#endif /* __ALPHABET_PHRASE_GENERATOR_H__ */
