#include <stdio.h>
#include "../common/ui.h"
#include "exhaustive_table_generator.h"

int main(int argc, char** argv) {
	const char * rule = NULL;
	const char * dictionaryFilename = NULL;
	const char * hashName = NULL;
	const char * prefix = NULL;
	const char * flag = NULL;
	
	if (6 != argc) {
		fprintf(stderr, "Error: Usage exhaustive_table_generator <rule> <dictionary> <hash> <filenames prefix> <\"all\" or number of random password>\n");
		return 1;
	}
	
	rule = argv[1];
	
	if (!validateRule(rule)) {
		return 1;
	}
	
	return 0;
}

int hashKeyIntoTable(const unsigned char * keyBuf,
		     int keySizeof,
		     int nTableSize);

int hashKeyforEfficientComparison(const unsigned char * keyBuf,
				  int keySizeof,
				  unsigned char *validationKeyBuf); 