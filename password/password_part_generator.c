#include <stdlib.h>
#include <string.h>
#include "alphabet_phrase_generator.h"
#include "mixed_cased_dictionary_word_generator.h"
#include "single_cased_dictionary_word_generator.h"
#include "password_part_generator.h"

static passwordPartGenerator_t * createAlphabetPhraseGenerator(const char * rangeStr, alphabet_t alphabet);
static passwordPartGenerator_t * createSingleCasedDictionaryWordGenerator(const dictionary_t * dictionary,
									  letterCase_t letterCase);
static passwordPartGenerator_t * createMixedCasedDictionaryWordGenerator(const dictionary_t * dictionary);

passwordPartGenerator_t * passwordPartGeneratorCreate(const char * rulePart, const dictionary_t * dictionary) {
	switch (rulePart[0]) {
	case '?':
		return createAlphabetPhraseGenerator(rulePart + 1, ALPHABET_PRINTABLE);
	case '&':
		return createAlphabetPhraseGenerator(rulePart + 1, ALPHABET_DIGIT);
	case '!':
		return createAlphabetPhraseGenerator(rulePart + 1, ALPHABET_ALPHA);
	case '%':
		return createAlphabetPhraseGenerator(rulePart + 1, ALPHABET_ALNUM);
	case '@':
		return createSingleCasedDictionaryWordGenerator(dictionary, LOWER_CASE);
	case '#':
		return createSingleCasedDictionaryWordGenerator(dictionary, UPPER_CASE);
	case '$':
		return createMixedCasedDictionaryWordGenerator(dictionary);
	}

	return NULL;
}

void passwordPartGeneratorInitialize(passwordPartGenerator_t * self) {
	self->size = 0;
	self->maxLength = 0;
	self->calcPassFunc = NULL;
	self->finalizeFunc = NULL;
}

void passwordPartGeneratorFinalize(passwordPartGenerator_t * self) {
	self->finalizeFunc(self);
}

ulong_t passwordPartGeneratorGetSize(const passwordPartGenerator_t * self) {
	return self->size;
}

ulong_t passwordPartGeneratorGetMaxLength(const passwordPartGenerator_t * self) {
	return self->maxLength;
}

char * passwordPartGeneratorCalculatePassword(const passwordPartGenerator_t * self,
					      ulong_t index,
					      char * buf) {
	return self->calcPassFunc(self, index, buf);
}

static passwordPartGenerator_t * createAlphabetPhraseGenerator(const char * rangeStr,
							       alphabet_t alphabet) {
	ulong_t minLength;
	ulong_t maxLength;
	alphabetPhraseGenerator_t * alphabetPhraseGenerator =
			malloc(sizeof(alphabetPhraseGenerator_t));

	if (NULL == alphabetPhraseGenerator) {
		return NULL;
	}

	minLength = atoi(rangeStr);
	maxLength = atoi(strchr(rangeStr, '-') + 1);

	alphabetPhraseGeneratorInitialize(alphabetPhraseGenerator, alphabet, minLength, maxLength);

	return (passwordPartGenerator_t *) alphabetPhraseGenerator;
}

static passwordPartGenerator_t * createSingleCasedDictionaryWordGenerator(const dictionary_t * dictionary,
									  letterCase_t letterCase) {
	singleCasedDictionaryWordGenerator_t * singleCasedDictionaryWordGenerator =
		malloc(sizeof(singleCasedDictionaryWordGenerator_t));

	if (NULL == singleCasedDictionaryWordGenerator) {
		return NULL;
	}

	singleCasedDictionaryWordGeneratorInitialize(singleCasedDictionaryWordGenerator,
						     dictionary,
						     letterCase);

	return (passwordPartGenerator_t *) singleCasedDictionaryWordGenerator;
}

static passwordPartGenerator_t * createMixedCasedDictionaryWordGenerator(const dictionary_t * dictionary) {
	mixedCasedDictionaryWordGenerator_t * mixedCasedDictionaryWordGenerator =
			malloc(sizeof(mixedCasedDictionaryWordGenerator_t));

	if (NULL == mixedCasedDictionaryWordGenerator) {
		return NULL;
	}

	mixedCasedDictionaryWordGeneratorInitialize(mixedCasedDictionaryWordGenerator,
						    dictionary);

	return (passwordPartGenerator_t *) mixedCasedDictionaryWordGenerator;
}
