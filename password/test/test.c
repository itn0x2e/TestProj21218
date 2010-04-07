#include <stdio.h>
#include <string.h>
#include "AlphabetTest.h"
#include "DictionaryTest.h"
#include "PasswordGeneratorTest.h"

int main(int argc, char** argv) {
	if (dictionaryTest()) {
	//if (alphabetTest()) {
	//if (generatorTest()) {
		printf("Test dictionary: SUCCESS\n");
	} else {
		printf("Test dictionary: FAILURE\n");
		return 1;
	}
	return 0;
}

