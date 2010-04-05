#include <stdio.h>
#include "../dictionary.h"
#include "../password_generator.h"
#include "PasswordGeneratorTest.h"

int generatorTest() {
	passwordGenerator_t pg;
	dictionary_t d;

	dictionaryInitialize(&d, "he\ni\nf@k3");
	passwordGeneratorInitialize(&pg, "||||||%0-2@||&1-1|||@@|||@||%0-2", &d);

	printf("Max length: %u\n", passwordGeneratorGetMaxLength(&pg));

	char buf[1000];
	uint_t i;

	for (i = 0; i < passwordGeneratorGetSize(&pg); ++i) {
		passwordGeneratorCalculatePassword(&pg, i, buf);
		printf("Password #%08u: %s\n", i, buf);
	}

	return TRUE;
}
