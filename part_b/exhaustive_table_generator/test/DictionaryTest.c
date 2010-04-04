/*
 * DictionaryTest.cpp
 *
 *  Created on: Apr 4, 2010
 *      Author: ritzpaz
 */
#include <ctype.h>
#include <string.h>
#include "../dictionary.h"
#include "../single_cased_dictionary_word_generator.h"
#include "DictionaryTest.h"

int isEmpty(const dictionary_t* dict);
int isGenEmpty(const dictionaryWordGenerator_t* dict);
int hasWords(const dictionary_t* dict, const char ** words, uint_t numWords);

int dictionaryTest() {
	int r = dictionaryTestEmpty() && dictionaryTestWords();
	return r;
}

int dictionaryTestEmpty() {
	dictionary_t emptyDict;
	if (!dictionaryInitialize(&emptyDict, "")) return 0;
	if (!isEmpty(&emptyDict)) return 0;
	dictionaryFinalize(&emptyDict);
	if (!dictionaryInitialize(&emptyDict, SPACES)) return 0;
	if (!isEmpty(&emptyDict)) return 0;

	singleCasedDictionaryWordGenerator_t gen;
	singleCasedDictionaryWordGeneratorInitialize(&gen, &emptyDict, LOWER_CASE);
	if (!isGenEmpty((dictionaryWordGenerator_t *) &gen)) return 0;
	singleCasedDictionaryWordGeneratorFinalize(&gen);

	singleCasedDictionaryWordGeneratorInitialize(&gen, &emptyDict, UPPER_CASE);
	if (!isGenEmpty((dictionaryWordGenerator_t *) &gen)) return 0;
	singleCasedDictionaryWordGeneratorFinalize(&gen);

	dictionaryFinalize(&emptyDict);

	return 1;
}

const char * words[] = {"hello",
							"woRlD",
							"\\/\\/0rLd",
							"__sieg$$__lk",
							"hello",
							"adsf^y7fd5%43",
							"LPOeo2nf34#DFdsv",
							"dsfkewL@L#@LLL",
							"hello",
							"adsf^y7fd5%43",
							"LPOeo2nf34#DFdsv",
							"dfle2@3fd","Dsfasdf","4kl2FFd", "dsfkl32"};

int dictionaryTestWords() {
	char r[5000] = {0};
	char * raw = r;
	int i, j;
	dictionary_t dict;
	for (i = 0; i < (sizeof(words) / sizeof(*words)); ++i) {
		for (j = 0; j < i % 2 + 1; ++j) {
			strcpy(raw, SPACES);
			raw += strlen(SPACES);
		}
		strcpy(raw, words[i]);
		raw += strlen(words[i]);
		for (j = 0; j < i % 3; ++j) {
			strcpy(raw, SPACES);
			raw += strlen(SPACES);
		}
		strcpy(raw, "\n");
		raw += 1;
	}

	if(!dictionaryInitialize(&dict, r)) return 0;
	if(!hasWords(&dict, words, (sizeof(words) / sizeof(*words)))) return 0;
	dictionaryFinalize(&dict);

	return 1;
}

int isEmpty(const dictionary_t* dict) {
	return ((dictionaryGetSize(dict) == 1) &&
			(dictionaryGetMaxLength(dict) == 0) &&
			(dictionaryGetEntry(dict, 0, LOWER_CASE) != NULL) &&
			(strlen(dictionaryGetEntry(dict, 0, LOWER_CASE)) == 0) &&
			(dictionaryGetEntry(dict, 0, UPPER_CASE) != NULL) &&
			(strlen(dictionaryGetEntry(dict, 0, UPPER_CASE)) == 0));
}

int isGenEmpty(const dictionaryWordGenerator_t* dict) {
	const passwordPartGenerator_t * gen = (const passwordPartGenerator_t *) dict;

	if ((passwordPartGeneratorGetSize(gen) != 1) ||(passwordPartGeneratorGetMaxLength(gen) != 0)) return FALSE;

	char buf[1000];
	buf[passwordPartGeneratorGetMaxLength(gen)] = '\0';

	if (passwordPartGeneratorCalculatePassword(gen, 0, buf) != buf) return FALSE;

	if (strlen(buf) != 0) return FALSE;

	return TRUE;
}

int hasWords(const dictionary_t* dict, const char ** words, uint_t numWords) {
	uint_t i, j;
	if (dictionaryGetSize(dict) != numWords + 1) return 0;

	singleCasedDictionaryWordGenerator_t lowgen;
	singleCasedDictionaryWordGeneratorInitialize(&lowgen, dict, LOWER_CASE);
	if (passwordPartGeneratorGetSize((const passwordPartGenerator_t *) &lowgen) != numWords + 1) return 0;

	singleCasedDictionaryWordGenerator_t upgen;
	singleCasedDictionaryWordGeneratorInitialize(&upgen, dict, UPPER_CASE);
	if (passwordPartGeneratorGetSize((const passwordPartGenerator_t *) &upgen) != numWords + 1) return 0;


	char calcLow[100];
	char calcUp[100];

	uint_t max = 0;

	for (i = 0; i < dictionaryGetSize(dict) - 1; ++i) {
		const char * low = dictionaryGetEntry(dict, i, LOWER_CASE);
		const char * up = dictionaryGetEntry(dict, i, UPPER_CASE);


		*passwordPartGeneratorCalculatePassword((const passwordPartGenerator_t *) &lowgen, i, calcLow) = '\0';
		*passwordPartGeneratorCalculatePassword((const passwordPartGenerator_t *) &upgen, i, calcUp) = '\0';

		if (strcmp(low, calcLow) != 0) return FALSE;
		if (strcmp(up, calcUp) != 0) return FALSE;

		if (strlen(low) != strlen(up)) return 0;
		for (j = 0; j < strlen(low); ++j) {
			if (isupper(low[j])) return 0;
			if (low[j] != tolower(words[i][j])) return 0;
			if (islower(up[j])) return 0;
			if (up[j] != toupper(words[i][j])) return 0;
		}

		if (strlen(low) > max) {
			max = strlen(low);
		}
	}

	if (strlen(dictionaryGetEntry(dict, numWords, LOWER_CASE)) != 0) return 0;
	if (strlen(dictionaryGetEntry(dict, numWords, UPPER_CASE)) != 0) return 0;
	if (passwordPartGeneratorCalculatePassword((const passwordPartGenerator_t *) &lowgen, numWords, calcLow) != calcLow) return FALSE;
	if (passwordPartGeneratorCalculatePassword((const passwordPartGenerator_t *) &upgen, numWords, calcUp) != calcUp) return FALSE;

	if (passwordPartGeneratorGetMaxLength((const passwordPartGenerator_t *) &lowgen) != max) return 0;
	if (passwordPartGeneratorGetMaxLength((const passwordPartGenerator_t *) &upgen) != max) return 0;
	if (dictionaryGetMaxLength(dict) != max) return 0;

	singleCasedDictionaryWordGeneratorFinalize(&upgen);
	singleCasedDictionaryWordGeneratorFinalize(&lowgen);

	return 1;
}
