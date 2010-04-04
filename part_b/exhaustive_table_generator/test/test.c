#include <stdio.h>
#include "DictionaryTest.h"

int main(int argc, char** argv) {
	if (dictionaryTest()) {
		printf("Test dictionary: SUCCESS\n");
	} else {
		printf("Test dictionary: FAILURE\n");
		return 1;
	}
	return 0;
}
