#include <math.h>
#include <string.h>
#include "../common/util.h"
#include "alphabet_phrase_generator.h"

static const char * ALPHABETS[] = {
	"0123456789",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
	" !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
};

void alphabetPhraseGeneratorInitialize(
		alphabetPhraseGenerator_t * self,
		alphabet_t alphabet,
		uint_t minLength,
		uint_t maxLength) {

	passwordPartGenerator_t * passwordPartGenerator = (passwordPartGenerator_t *) self;

	ASSERT(minLength <= maxLength);

	passwordPartGeneratorInitialize(passwordPartGenerator);

	self->alphabetChars = ALPHABETS[alphabet];
	self->alphabetLength = strlen(self->alphabetChars);

	/* The start index is the number of possible phrases shorter than minLength */
	if (0 == minLength) {
		self->startIndex = 0;
	} else {
		self->startIndex = (pow(self->alphabetLength, minLength) - 1) / (self->alphabetLength - 1);
	}

	/* The size is the number of possible phrases shorter than (maxLength + 1),
	 * except those shorter than minLength */
	passwordPartGenerator->size = (pow(self->alphabetLength, maxLength + 1) - 1) / (self->alphabetLength - 1);
	passwordPartGenerator->size -= self->startIndex;

	passwordPartGenerator->maxLength = maxLength;
	passwordPartGenerator->calcPassFunc =
			(passwordPartGeneratorCalcPassFuncPtr_t)
			alphabetPhraseGeneratorCalcPass;
	passwordPartGenerator->finalizeFunc =
			(passwordPartGeneratorFinalizeFuncPtr_t)
			alphabetPhraseGeneratorFinalize;
}

void alphabetPhraseGeneratorFinalize(alphabetPhraseGenerator_t * self) {
	/* Left blank on purpose */
}

char * alphabetPhraseGeneratorCalcPass(const alphabetPhraseGenerator_t * self, ulong_t index, char * buf) {
	ASSERT(index < ((passwordPartGenerator_t *) self)->size);

	index += self->startIndex;

	while (index > 0) {
		index -= 1;

		*buf = self->alphabetChars[index % self->alphabetLength];
		++buf;

		index /= self->alphabetLength;
	}

	return buf;
}
