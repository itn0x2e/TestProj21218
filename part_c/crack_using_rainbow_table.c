#include <stdio.h>
#include <string.h>
#include "../common/constants.h"
#include "../common/io.h"
#include "../common/ui.h"
#include "../password/dictionary.h"
#include "../password/password_generator.h"
#include "../rainbow_table/rainbow_table.h"

void commandLoop(RainbowTable_t * rainbowTable, char * password);
RainbowTable_t * initializeRainbowTable(const passwordGenerator_t * passGenerator,
					  char * generatorPassword,
					  const char * prefix);
void finalizeRainbowTable(RainbowTable_t * rainbowTable); 
bool_t queryRainbowTable(RainbowTable_t * rainbowTable, byte_t * hash, uint_t hashLen, bool_t * found); 

int main(int argc, char** argv);

bool_t crackUsingRainbowTable(const char * prefix,
			      const char * rule,
			      const char * dictionaryFilename);

bool_t crackUsingRainbowTableUsingIni(const char * prefix, const char * iniFilename);

int main(int argc, char** argv) {
	if (3 != argc) {
		fprintf(stderr, "Error: Usage crack_using_rainbow_table <filenames prefix> <preferences filename>\n");
		return 1;
	}
	
	if (crackUsingRainbowTableUsingIni(argv[1], argv[2])) {
		return 0;
	}
	
	return 1;
}

bool_t crackUsingRainbowTable(const char * prefix,
			      const char * rule,
			      const char * dictionaryFilename) {
	bool_t ret = FALSE;
	dictionary_t dictionary;
	passwordGenerator_t passwordGenerator;
	ulong_t maxPasswordLength;
	char * generatorPassword = NULL;
	RainbowTable_t * rainbowTable = NULL;
	
	CHECK(verifyDEHTExists(prefix));
	CHECK(validateRule(rule));
	CHECK(readDictionaryFromFile(&dictionary, dictionaryFilename));
	
	if (!passwordGeneratorInitialize(&passwordGenerator, rule, &dictionary)) {
		goto LBL_CLEANUP_DICTIONARY;
	}
	
	maxPasswordLength = passwordGeneratorGetMaxLength(&passwordGenerator);
	
	generatorPassword = (char *) malloc((maxPasswordLength + 1) * sizeof(char));
	if (NULL == generatorPassword) {
		PERROR();
		goto LBL_CLEANUP_GENERATOR;
	}
	
	rainbowTable = initializeRainbowTable(&passwordGenerator, generatorPassword, prefix);
	if (NULL == rainbowTable) {
		goto LBL_CLEANUP_GENERATOR;
	}
	
	commandLoop(rainbowTable, generatorPassword);
	
	finalizeRainbowTable(rainbowTable);
	ret = TRUE;

LBL_CLEANUP_GENERATOR:
	passwordGeneratorFinalize(&passwordGenerator);
LBL_CLEANUP_DICTIONARY:
	dictionaryFinalize(&dictionary);
LBL_ERROR:
	FREE(generatorPassword);
	return ret;
}

bool_t crackUsingRainbowTableUsingIni(const char * prefix, const char * iniFilename) {
	bool_t ret = FALSE;
	const char * keys[] = { "rule", "dictionary_name" };
	const char * values[sizeof(keys) / sizeof(*keys)] = {0};
	const uint_t numKeys = sizeof(keys) / sizeof(*keys);
	char * iniContent = NULL;
	
	/* Parse INI file */
	CHECK(verifyFileExists(iniFilename));
	iniContent = readEntireTextFile(iniFilename);
	CHECK(NULL != iniContent);
	CHECK(parseIni(iniContent, keys, values, numKeys));
	
	/* Invoke rainbow table creation */
	ret = crackUsingRainbowTable(prefix, values[0], values[1]);

LBL_ERROR:
	FREE(iniContent);
	return ret;
}

void commandLoop(RainbowTable_t * rainbowTable, char * password) {
	char line[MAX_LINE_LEN] = {0};
	byte_t hash[MAX_LINE_LEN] = {0}; /* The hash is never longer than its string representation */
	uint_t hashLen = 0;		

	/* Treat the user's submitted requests until told to quit */
	for(;;) {
		bool_t found = FALSE;

		if (!readPrompt(line) || (0 == strcmp(line, "quit"))) {
			return;
		}
		
		if ('!' == line[0]) {
			char hashStr[MAX_DIGEST_LEN * 2 + 1];
			
			hashLen = (uint_t) cryptHash(rainbowTable->hashFunc, line + 1, hash);
			binary2hexa(hash, hashLen, hashStr, sizeof(hashStr));
			printf("\tIn hexa password is%s\n", hashStr);
		} else {
			hashLen = (uint_t) hexa2binary(line, hash, sizeof(hash));
			if (-1 == hashLen) {
				fprintf(stderr, "Commands are either hexa, !password or quit.\n");
				continue;
			}
		}

		if (!queryRainbowTable(rainbowTable, hash, hashLen, &found)) {
			/* An error has occured and a message has already been printed */
			return;
		}

		if (found) {
			printf("Try to login with password \"%s\"\n", password);
		} else {
			printf("Sorry but this hash doesn't appears in pre-processing\n");
		}
	}
}

RainbowTable_t * initializeRainbowTable(const passwordGenerator_t * passGenerator,
					char * generatorPassword,
					const char * prefix) {
	return RT_open(passGenerator,
			generatorPassword,
			passwordGeneratorGetMaxLength(passGenerator),
			prefix,
			TRUE);
}

void finalizeRainbowTable(RainbowTable_t * rainbowTable) {
	RT_close(rainbowTable);
}

bool_t queryRainbowTable(RainbowTable_t * rainbowTable, byte_t * hash, uint_t hashLen, bool_t * found) {
	return RT_query(rainbowTable, hash, hashLen, found);
}
