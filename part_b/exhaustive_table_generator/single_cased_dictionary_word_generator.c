#include <string.h>
#include "../../common/util.h"
#include "single_cased_dictionary_word_generator.h"

inline void singleCasedDictionaryWordGeneratorInitialize(
		singleCasedDictionaryWordGenerator_t * self,
		const dictionary_t * dictionary,
		letterCase_t letterCase) {

	passwordPartGenerator_t * passwordPartGenerator = (passwordPartGenerator_t *) self;

	dictionaryWordGeneratorInitialize((dictionaryWordGenerator_t *) self, dictionary);
	self->letterCase = letterCase;

	passwordPartGenerator->size = dictionaryGetSize(dictionary);
	passwordPartGenerator->maxLength = dictionaryGetMaxLength(dictionary);
	passwordPartGenerator->calcPassFunc =
			(passwordPartGeneratorCalcPassFuncPtr_t)
			singleCasedDictionaryWordGeneratorCalcPass;
	passwordPartGenerator->finalizeFunc =
			(passwordPartGeneratorFinalizeFuncPtr_t)
			singleCasedDictionaryWordGeneratorFinalize;
}

inline bool_t singleCasedDictionaryWordGeneratorFinalize(singleCasedDictionaryWordGenerator_t * singleCasedDictionaryWordGenerator) {
	return TRUE;
}

char * singleCasedDictionaryWordGeneratorCalcPass(
		const singleCasedDictionaryWordGenerator_t * self,
		ulong_t index,
		char * buf) {
	const dictionary_t * dictionary = ((dictionaryWordGenerator_t *) self)->dictionary;
	const char * dictionaryEntry = NULL;

	ASSERT(index < dictionaryGetSize(dictionary));

	dictionaryEntry = dictionaryGetEntry(dictionary, index, self->letterCase);
	assert(NULL != dictionaryEntry);

	strcpy(buf, dictionaryEntry);

	return buf + strlen(dictionaryEntry);
}
