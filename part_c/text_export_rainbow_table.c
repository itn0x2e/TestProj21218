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

	/*! TODO: we only need dictionary name and rule, don't we? !*/
	const char * keys[] = { "dictionary_name", "rainbow_chain_length", "entires_in_hash_table", "bucket_block_length", "rule" };
	const char * values[sizeof(keys) / sizeof(*keys)] = {0};
	const uint_t numKeys = sizeof(keys) / sizeof(*keys);
	char * iniContent = NULL;
	FILE * output1 = NULL;
	FILE * output2 = NULL;

	const char * rule = NULL;
	const char * dictionaryFilename = NULL;

	dictionary_t dictionary;
	passwordGenerator_t passwordGenerator;
	ulong_t maxPasswordLength;
	char * generatorPassword = NULL;

	TRACE_FUNC_ENTRY();
	
	CHECK(NULL != prefix);
	CHECK(NULL != iniFilename);
	CHECK(NULL != output1Filename);
	CHECK(NULL != output2Filename);

	
	/* Verify that the DEHT files exist and the output files don't */
	CHECK(verifyDEHTExists(prefix));
	CHECK(verifyFileNotExist(output1Filename));
	CHECK(verifyFileNotExist(output2Filename));
	
	/* Parse INI file and print it. We allow missing entries */
	CHECK(verifyFileExists(iniFilename));
	iniContent = readEntireTextFile(iniFilename);
	CHECK(NULL != iniContent);
	CHECK(parseIniPartial(iniContent, keys, values, numKeys));

	printIni(keys, values, numKeys);

	/*! TODO: better way of doing this? !*/
	dictionaryFilename = values[0];
	CHECK(NULL != dictionaryFilename);
	rule = values[4];
	CHECK(NULL != rule);
	
	CHECK(validateRule(rule));
	CHECK(readDictionaryFromFile(&dictionary, dictionaryFilename));
	
	CHECK(passwordGeneratorInitialize(&passwordGenerator, rule, &dictionary));
	
	maxPasswordLength = passwordGeneratorGetMaxLength(&passwordGenerator);
	
	generatorPassword = (char *) malloc((maxPasswordLength + 1) * sizeof(char));
	CHECK_MSG("malloc", (NULL != generatorPassword));


	passwordGeneratorCalculatePassword(&passwordGenerator, 0, generatorPassword);
	printf("pass 0=%s\n", generatorPassword);
	passwordGeneratorCalculatePassword(&passwordGenerator, 1, generatorPassword);
	printf("pass 1=%s\n", generatorPassword);
	passwordGeneratorCalculatePassword(&passwordGenerator, 2, generatorPassword);
	printf("pass 2=%s\n", generatorPassword);

	/* Open output files for writing */
	output1 = fopen(output1Filename, "w");
	CHECK_MSG(output1Filename, (NULL != output1));
	output2 = fopen(output2Filename, "w");
	CHECK_MSG(output2Filename, (NULL != output2));
	
	/* Scan DEHT file and print its content */
	CHECK(RT_print(output1, output2, 
		       &passwordGenerator, generatorPassword, maxPasswordLength,
		       prefix));

	ret = TRUE;
	goto LBL_CLEANUP;
	
LBL_ERROR:
	TRACE_FUNC_ERROR();

	/*! TODO: delete files on error? !*/
	ret = FALSE;

LBL_CLEANUP:
	FREE(generatorPassword);
	FREE(iniContent);
	FCLOSE(output1);
	FCLOSE(output2);

	TRACE_FUNC_EXIT();
	return ret;
}
