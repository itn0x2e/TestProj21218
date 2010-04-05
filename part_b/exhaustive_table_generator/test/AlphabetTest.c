#include <stdio.h>
#include "../alphabet_phrase_generator.h"
#include "AlphabetTest.h"

int alphabetTest() {
	alphabetPhraseGenerator_t gen;
	passwordPartGenerator_t * ppg = (passwordPartGenerator_t *) &gen;
	alphabetPhraseGeneratorInitialize(&gen, ALPHABET_ALPHA, 2, 5);

	uint_t i;
	char buf[100];
	printf("Max length: %u\n", passwordPartGeneratorGetMaxLength(ppg));
	printf("Size: %u\n", passwordPartGeneratorGetSize(ppg));
	for (i = 0; i < passwordPartGeneratorGetSize(ppg); ++i) {
		*passwordPartGeneratorCalculatePassword(ppg, i, buf) = '\0';
		printf("Password #%08u: %s\n", i, buf);
	}

	alphabetPhraseGeneratorFinalize(&gen);

	return TRUE;
}
