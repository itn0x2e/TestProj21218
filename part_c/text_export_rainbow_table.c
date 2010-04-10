#include <stdio.h>
#include <string.h>
#include "../common/io.h"
#include "../common/ui.h"
#include "../rainbow_table/rainbow_table.h"

int main(int argc, char** argv);
bool_t textExportRainbowTable(const char * prefix,
			      const char * iniFilename,
			      const char * output1Filename,
			      const char * output2Filename);

int main(int argc, char** argv) {
	if (5 != argc) {
		fprintf(stderr, "Error: Usage text_export_rainbow_table <filenames prefix> <preferences filename> <output 1 filename> <output 2 filename>\n");
		return 1;
	}
	
	if (textExportRainbowTable(argv[1], argv[2], argv[3], argv[4])) {
		return 0;
	}
	
	return 1;
}

bool_t textExportRainbowTable(const char * prefix,
			      const char * iniFilename,
			      const char * output1Filename,
			      const char * output2Filename) {
	bool_t ret = FALSE;
	const char * keys[] = { "rule", "dictionary_name" };
	const char * values[sizeof(keys) / sizeof(*keys)] = {0};
	const uint_t numKeys = sizeof(keys) / sizeof(*keys);
	dictionary_t dictionary;
	char * iniContent = NULL;
	passwordGenerator_t passwordGenerator;
	ulong_t maxPasswordLength;
	char * generatorPassword = NULL;
	FILE * output1 = NULL;
	FILE * output2 = NULL;
	
	/* Verify that the DEHT files exist and the output files don't */
	CHECK(verifyDEHTExists(prefix));
	CHECK(verifyFileNotExist(output1Filename));
	CHECK(verifyFileNotExist(output2Filename));
	
	/* Parse INI file and print it. We allow missing entries */
	CHECK(verifyFileExists(iniFilename));
	iniContent = readEntireTextFile(iniFilename);
	CHECK(NULL != iniContent);
	CHECK(parseIni(iniContent, keys, values, numKeys));
	printIni(keys, values, numKeys);
	
	CHECK(validateRule(values[0]));
	CHECK(readDictionaryFromFile(&dictionary, values[1]));

	if (!passwordGeneratorInitialize(&passwordGenerator, rule, &dictionary)) {
		goto LBL_CLEANUP_DICTIONARY;
	}

	maxPasswordLength = passwordGeneratorGetMaxLength(&passwordGenerator);

	generatorPassword = (char *) malloc((maxPasswordLength + 1) * sizeof(char));
	if (NULL == generatorPassword) {
		PERROR();
		goto LBL_CLEANUP_GENERATOR;
	}

	/* Open output files for writing */
	output1 = fopen(output1Filename, "w");
	if (NULL == output1) {
		perror(output1Filename);
		goto LBL_CLEANUP_GENERATOR;
	}
	output2 = fopen(output1Filename, "w");
	if (NULL == output2) {
		perror(output2Filename);
		goto LBL_CLEANUP_GENERATOR;
	}
	
	/* Scan DEHT file and print its content */
	ret = RT_print(output1, output2, passwordGenerator, generatorPassword, prefix);

LBL_CLEANUP_GENERATOR:
	passwordGeneratorFinalize(&passwordGenerator);
LBL_CLEANUP_DICTIONARY:
	dictionaryFinalize(&dictionary);
LBL_ERROR:
	FREE(generatorPassword);
	FREE(iniContent);
	FCLOSE(output1);
	FCLOSE(output2);
	return ret;
}
