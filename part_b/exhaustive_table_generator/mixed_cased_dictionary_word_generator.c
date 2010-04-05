#include <ctype.h>
#include <string.h>
#include "../../common/util.h"
#include "mixed_cased_dictionary_word_generator.h"

static inline ulong_t countLower(const char * str);
static inline ulong_t getNumOfVariations(const char * str);
static void applyStringCapitalizationMask(char * str, ulong_t mask);
static inline char * getNextLower(char * str);

inline bool_t mixedCasedDictionaryWordGeneratorInitialize(
		mixedCasedDictionaryWordGenerator_t * self,
		const dictionary_t * dictionary) {

	passwordPartGenerator_t * passwordPartGenerator = (passwordPartGenerator_t *) self;
	ulong_t dictionarySize = dictionaryGetSize(dictionary);
	ulong_t i;

	dictionaryWordGeneratorInitialize((dictionaryWordGenerator_t *) self, dictionary);

	passwordPartGenerator->maxLength = dictionaryGetMaxLength(dictionary);
	passwordPartGenerator->calcPassFunc =
			(passwordPartGeneratorCalcPassFuncPtr_t)
			mixedCasedDictionaryWordGeneratorCalcPass;
	passwordPartGenerator->finalizeFunc =
			(passwordPartGeneratorFinalizeFuncPtr_t)
			mixedCasedDictionaryWordGeneratorFinalize;

	if (0 == dictionarySize) {
		passwordPartGenerator->size = 0;
		self->accumulativeSizes = NULL;
		return TRUE;
	}

	self->accumulativeSizes = (ulong_t *) malloc(dictionarySize * sizeof(ulong_t));
	CHECK(NULL != self->accumulativeSizes);

	self->accumulativeSizes[0] = getNumOfVariations(dictionaryGetEntry(dictionary, 0, LOWER_CASE));

	for (i = 1; i < dictionarySize; ++i) {
		self->accumulativeSizes[i] =
				self->accumulativeSizes[i - 1] +
				getNumOfVariations(dictionaryGetEntry(dictionary, i, LOWER_CASE));
	}

	passwordPartGenerator->size = self->accumulativeSizes[dictionarySize - 1];

	return TRUE;

LBL_ERROR:
	return FALSE;
}

inline void mixedCasedDictionaryWordGeneratorFinalize(mixedCasedDictionaryWordGenerator_t * self) {
	FREE(self->accumulativeSizes);
}

char * mixedCasedDictionaryWordGeneratorCalcPass(
		const mixedCasedDictionaryWordGenerator_t * self,
		ulong_t index,
		char * buf) {
	const dictionary_t * dictionary = ((dictionaryWordGenerator_t *) self)->dictionary;
	const char * dictionaryEntry = NULL;
	ulong_t size = ((passwordPartGenerator_t *) self)->size;
	ulong_t i;

	ASSERT(index < size);

	/* TODO: consider binary search */
	for (i = 0; i < size; ++i) {
		if (index < self->accumulativeSizes[i]) {
			ulong_t capitalizationMask;

			dictionaryEntry = dictionaryGetEntry(dictionary, i, LOWER_CASE);
			ASSERT(NULL != dictionaryEntry);

			strcpy(buf, dictionaryEntry);

			if (0 == i) {
				capitalizationMask = index;
			} else {
				/* The mask is equal to the index within the range of variations of the current word */
				capitalizationMask = index - self->accumulativeSizes[i - 1];
			}

			applyStringCapitalizationMask(buf, capitalizationMask);

			return buf + strlen(dictionaryEntry);
		}

	}

	/*This should never be reached */
	ASSERT(FALSE);
	return buf;
}

static inline ulong_t countLower(const char * str) {
	ulong_t len = strlen(str);
	ulong_t count = 0;
	ulong_t i;

	for (i = 0; i < len; ++i) {
		if (islower(str[i])) {
			++count;
		}
	}

	return count;
}

static inline ulong_t getNumOfVariations(const char * str) {
	return (1 << countLower(str));
}

static void applyStringCapitalizationMask(char * str, ulong_t mask) {
	/* TODO: document that it is assumed that the mask does not exceed the length of the string */

	while (mask > 0) {
		/* Find the next lower-cased symbol */
		str = getNextLower(str);

		/* If masked, convert it to upper case */
		*str -= 0x20 * (mask & 1);

		/* Continue with the rest of the string and the rest of the mask */
		++str;
		mask >>= 1;
	}
}

static inline char * getNextLower(char * str) {
	/* TODO: document that we assume that such exists */

	while (!islower(*str)) {
		++str;
	}

	return str;
}
