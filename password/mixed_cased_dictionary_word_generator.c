#include <ctype.h>
#include <string.h>
#include "../common/utils.h"
#include "mixed_cased_dictionary_word_generator.h"

/**
 * @param str	A null-terminated string
 * @return The number of lower case characters in the string
 */
static ulong_t countLower(const char * str);

/**
 * @param str	A null-terminated string
 * @return The number of case variation possibilities of the string
 */
static ulong_t getNumOfVariations(const char * str);

/**
 * Turns some of the lower cased characters in a string into upper cased
 * according to a bitmask.
 *
 * @param str	A null-terminated string
 * @param mask	A bitmask in which the bits set represent the indexes
 *		(refering to the lower case characters only) in the
 *		string which should be capitalized.
 *
 * @pre The mask does not exceed the length of the string.
 */
static void applyStringCapitalizationMask(char * str, ulong_t mask);

/**
 * @param str	A null-terminated string
 * @return 	A pointer to the next lower cased character in the string.
 *
 * @pre Such a character exists.
 */
static char * getNextLower(char * str);

/* A comparator for bsearch which compares an index to a range from the
 * precomputed range array */
static int compareIndexToRange(const void * a, const void * b);

bool_t mixedCasedDictionaryWordGeneratorInitialize(
		mixedCasedDictionaryWordGenerator_t * self,
		const dictionary_t * dictionary) {

	passwordPartGenerator_t * passwordPartGenerator = (passwordPartGenerator_t *) self;
	ulong_t dictionarySize = dictionaryGetSize(dictionary);
	ulong_t i;

	dictionaryWordGeneratorInitialize((dictionaryWordGenerator_t *) self, dictionary);

	passwordPartGenerator->maxLength = dictionaryGetMaxLength(dictionary);
	passwordPartGenerator->calcPassFunc =
		(passwordPartGeneratorCalcPassFuncPtr_t) mixedCasedDictionaryWordGeneratorCalcPass;
	passwordPartGenerator->finalizeFunc =
		(passwordPartGeneratorFinalizeFuncPtr_t) mixedCasedDictionaryWordGeneratorFinalize;

	if (0 == dictionarySize) {
		passwordPartGenerator->size = 0;
		self->beginIndexes = NULL;
		return TRUE;
	}

	self->beginIndexes = (ulong_t *) malloc((dictionarySize + 1)* sizeof(ulong_t));
	CHECK(NULL != self->beginIndexes);

	self->beginIndexes[0] = 0;

	for (i = 0; i < dictionarySize; ++i) {
		self->beginIndexes[i + 1] =
				self->beginIndexes[i] +
				getNumOfVariations(dictionaryGetEntry(dictionary, i, LOWER_CASE));
	}

	passwordPartGenerator->size = self->beginIndexes[dictionarySize];

	return TRUE;

LBL_ERROR:
	return FALSE;
}

void mixedCasedDictionaryWordGeneratorFinalize(mixedCasedDictionaryWordGenerator_t * self) {
	FREE(self->beginIndexes);
}

char * mixedCasedDictionaryWordGeneratorCalcPass(
		const mixedCasedDictionaryWordGenerator_t * self,
		ulong_t index,
		char * buf) {
	const dictionary_t * dictionary = ((dictionaryWordGenerator_t *) self)->dictionary;
	const char * dictionaryEntry = NULL;
	ulong_t size = dictionaryGetSize(dictionary);
	ulong_t * pItem = (ulong_t *) bsearch(&index, self->beginIndexes, size, sizeof (ulong_t), compareIndexToRange);
	ulong_t i = (ulong_t) (pItem - self->beginIndexes);
	ulong_t capitalizationMask;

	dictionaryEntry = dictionaryGetEntry(dictionary, i, LOWER_CASE);
	ASSERT(NULL != dictionaryEntry);

	strcpy(buf, dictionaryEntry);

	/* The mask is equal to the index within the range of variations of the current word */
	capitalizationMask = index - self->beginIndexes[i];
	applyStringCapitalizationMask(buf, capitalizationMask);

	return buf + strlen(dictionaryEntry);
}

static ulong_t countLower(const char * str) {
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

static ulong_t getNumOfVariations(const char * str) {
	return (1 << countLower(str));
}

static void applyStringCapitalizationMask(char * str, ulong_t mask) {
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

static char * getNextLower(char * str) {
	while (!islower(*str)) {
		++str;
	}

	return str;
}

static int compareIndexToRange(const void * a, const void * b) {
	const ulong_t index = *((const ulong_t *) a);

	/* A pointer to the first index within the range */
	const ulong_t * pBeginIndex = (const ulong_t *) b;

	/* A pointer to the first index which is after the range */
	const ulong_t * pEndIndex = pBeginIndex + 1;

	ASSERT(*pBeginIndex <= *pEndIndex);

	if (index >= *pEndIndex) {
		return 1;
	}

	if (index < *pBeginIndex) {
		return -1;
	}

	return 0;
}
