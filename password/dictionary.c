#include <ctype.h>
#include <string.h>
#include "../common/types.h"
#include "../common/utils.h"
#include "dictionary.h"

static bool_t findNextWordInRawDict(char * rawDict, char ** next, uint_t * nextLen);
static void strToLower(char * buf, uint_t len);
static void strToUpper(char * buf, uint_t len);

bool_t dictionaryInitialize(dictionary_t * self, const char * rawDict) {
	uint_t i = 0;
	uint_t dictLen = 0;
	char * currWord;
	uint_t currWordLen = 0;
	uint_t rawDictLen = strlen(rawDict);

	self->rawDataLowerUpper = (char *) malloc(((2 * rawDictLen) + 1) * sizeof(*(self->rawDataLowerUpper)));
	CHECK(NULL != self->rawDataLowerUpper);

	strcpy(self->rawDataLowerUpper, rawDict);
	strToLower(self->rawDataLowerUpper, rawDictLen);
	strcpy(self->rawDataLowerUpper + rawDictLen, rawDict);
	strToUpper(self->rawDataLowerUpper + rawDictLen, rawDictLen);
	self->rawDataLowerUpper[2 * rawDictLen] = '\0';

	currWord = self->rawDataLowerUpper + rawDictLen;

	while (findNextWordInRawDict(currWord, &currWord, &currWordLen)) {
		++dictLen;
		currWord += currWordLen + 1;
	}

	/* Alloc the vector of string pointer pairs. We alloc one extra member for a null terminator */
	self->entries = (stringPair_t *) calloc(dictLen + 1, sizeof(*(self->entries)));
	CHECK(NULL != self->entries);
	self->numEntries = dictLen + 1;
	self->maxEntryLength = 0;

	/* Now scan the raw dict again, this time updating the resulting array and splitting the strings */
	currWord = self->rawDataLowerUpper + rawDictLen;
	findNextWordInRawDict(currWord, &currWord, &currWordLen);

	for (i = 0; i < dictLen; ++i) {
		findNextWordInRawDict(currWord, &currWord, &currWordLen);

		self->entries[i][LOWER_CASE] = currWord - rawDictLen;
		self->entries[i][LOWER_CASE][currWordLen] = '\0';
		self->entries[i][UPPER_CASE] = currWord;
		self->entries[i][UPPER_CASE][currWordLen] = '\0';

		if (self->maxEntryLength < currWordLen) {
			self->maxEntryLength = currWordLen;
		}

		currWord += currWordLen + 1;
	}

	/* Every dictionary contains the empty word, so the last entry points to '\0' */
	self->entries[i][LOWER_CASE] = self->rawDataLowerUpper + (2 * rawDictLen);
	self->entries[i][UPPER_CASE] = self->rawDataLowerUpper + (2 * rawDictLen);

	return TRUE;

LBL_ERROR:
	PERROR();
	FREE(self->rawDataLowerUpper);
	return FALSE;
}

void dictionaryFinalize(dictionary_t * self) {
	FREE(self->entries);
	FREE(self->rawDataLowerUpper);
}

ulong_t dictionaryGetSize(const dictionary_t * self) {
	return self->numEntries;
}

ulong_t dictionaryGetMaxLength(const dictionary_t * self) {
	return self->maxEntryLength;
}

const char * dictionaryGetEntry(const dictionary_t * self, ulong_t index, letterCase_t letterCase) {
	ASSERT(index < self->numEntries);
	return self->entries[index][letterCase];
}

static bool_t findNextWordInRawDict(char * rawDict, char ** next, uint_t * nextLen) {
	uint_t i = 0;

	/* Skip any loose terminators in the string's beginning */
	while (('\0' != *rawDict) && !isgraph(*rawDict)) {
		++rawDict;
	}

	/* If the end of the raw dictionary is reached before any word is found, return FALSE */
	if ('\0' == *rawDict) {
		return FALSE;
	}

	/* Scan the string, looking for the word's ending.
	 * Since ' ' is considered a valid symbol in passwords, it is also considered
	 * a valid symbol in the middle or ending of a dictionary entry. */
	for (i = 0; ('\0' != rawDict[i]) && (isgraph(rawDict[i]) || (' ' == rawDict[i])); ++i) {
		/* Do nothing here */
	}

	*next = rawDict;
	*nextLen = i;

	return TRUE;
}

static void strToLower(char * buf, uint_t len) {
	uint_t i = 0;

	for (i = 0; i < len; ++i) {
		buf[i] = tolower(buf[i]);
	}
}

static void strToUpper(char * buf, uint_t len) {
	uint_t i = 0;

	for (i = 0; i < len; ++i) {
		buf[i] = toupper(buf[i]);
	}
}
