#include <stddef.h>
#include "password_part_generator.h"

static passwordPartGenerator_t * createSingleCasedDictionaryWordGenerator(const char * rulePart, const dictionary_t * dictionary, letterCase_t letterCase);
static passwordPartGenerator_t * createMixedCasedDictionaryWordGenerator(const char * rulePart, const dictionary_t * dictionary);

passwordPartGenerator_t * passwordPartGeneratorCreate(const char * rulePart, const dictionary_t * dictionary) {
	passwordPartGenerator_t * newGenerator;

	switch (rulePart[0]) {
	case '@':
		newGenerator = createSingleCasedDictionaryWordGenerator(rulePart, dictionary, LOWER_CASE);
	case '#':
		newGenerator = createSingleCasedDictionaryWordGenerator(rulePart, dictionary, UPPER_CASE);
	case '$':
		newGenerator = createMixedCasedDictionaryWordGenerator(rulePart, dictionary);
	default:
		newGenerator = NULL;
	}

	return newGenerator;
}

inline void passwordPartGeneratorInitialize(passwordPartGenerator_t * self) {
	self->size = 0;
	self->maxLength = 0;
	self->calcPassFunc = NULL;
	self->finalizeFunc = NULL;
}

inline void passwordPartGeneratorFinalize(passwordPartGenerator_t * self) {
	self->finalizeFunc(self);
}

inline ulong_t passwordPartGeneratorGetSize(const passwordPartGenerator_t * self) {
	return self->size;
}

inline ulong_t passwordPartGeneratorGetMaxLength(const passwordPartGenerator_t * self) {
	return self->maxLength;
}

inline char * passwordPartGeneratorCalculatePassword(const passwordPartGenerator_t * self, ulong_t index, char * buf) {
	return self->calcPassFunc(self, index, buf);
}
