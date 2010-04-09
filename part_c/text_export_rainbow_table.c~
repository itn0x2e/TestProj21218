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
	const char * keys[] = { "dictionary_name", "rainbow_chain_length", "entires_in_hash_table", "bucket_block_length", "rule" };
	const char * values[sizeof(keys) / sizeof(*keys)] = {0};
	const uint_t numKeys = sizeof(keys) / sizeof(*keys);
	char * iniContent = NULL;
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
	CHECK(parseIniPartial(iniContent, keys, values, numKeys));
	printIni(keys, values, numKeys);
	
	/* Open output files for writing */
	output1 = fopen(output1Filename, "w");
	if (NULL == output1) {
		perror(output1Filename);
		goto LBL_ERROR;
	}
	output2 = fopen(output1Filename, "w");
	if (NULL == output2) {
		perror(output2Filename);
		goto LBL_ERROR;
	}
	
	/* Scan DEHT file and print its content */
	ret = RT_print(prefix, output1, output2);
	
LBL_ERROR:
	FREE(iniContent);
	FCLOSE(output1);
	FCLOSE(output2);
	return ret;
}
