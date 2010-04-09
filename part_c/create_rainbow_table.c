#include <stdio.h>
#include <string.h>
#include "../common/io.h"
#include "../common/ui.h"
#include "../password/dictionary.h"
#include "../password/password_enumerator.h"
#include "../password/password_generator.h"
#include "../password/random_password_enumerator.h"
#include "../rainbow_table/rainbow_table.h"

#define RANDOM_PASSWORD_NUM_FACTOR (10)

int main(int argc, char** argv);

bool_t createRainbowTable(const char * hashName,
			  const char * prefix,
			  const char * rule,
			  const char * dictionaryFilename,
			  const char * chainLengthStr,
			  const char * entiresInHashTableStr,
			  const char * bucketBlockLengthStr);

bool_t createRainbowTableUsingIni(const char * hashName,
			    const char * prefix,
			    const char * rule,
			    const char * iniFilename);

int main(int argc, char** argv) {
	if (5 != argc) {
		fprintf(stderr, "Error: Usage create_rainbow_table <cryptographic hash> "
			"<output filenames prefix> <rule> <preferences filename>\n");
		return 1;
	}
	
	if (createRainbowTableUsingIni(argv[1], argv[2], argv[3], argv[4])) {
		return 0;
	}
	
	return 1;
}

bool_t createRainbowTable(const char * hashName,
			  const char * prefix,
			  const char * rule,
			  const char * dictionaryFilename,
			  const char * chainLengthStr,
			  const char * entiresInHashTableStr,
			  const char * bucketBlockLengthStr) {
	bool_t ret = FALSE;
	BasicHashFunctionPtr hashFunc;
	ulong_t chainLength;
	ulong_t entiresInHashTable;
	ulong_t bucketBlockLength;
	dictionary_t dictionary;
	passwordGenerator_t passwordGenerator;
	ulong_t maxPasswordLength;
	char * enumeratorPassword = NULL;
	char * generatorPassword = NULL;
	randomPasswordEnumerator_t randomPasswordEnumerator;
	
	CHECK(parseHashFunName(&hashFunc, hashName));
	CHECK(verifyDEHTNotExist(prefix));
	CHECK(validateRule(rule));
	CHECK(parseIniNum(chainLengthStr, &chainLength));
	CHECK(parseIniNum(entiresInHashTableStr, &entiresInHashTable));
	CHECK(parseIniNum(bucketBlockLengthStr, &bucketBlockLength));
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
	
	enumeratorPassword = (char *) malloc((maxPasswordLength + 1) * sizeof(char));
	if (NULL == enumeratorPassword) {
		PERROR();
		goto LBL_CLEANUP_GENERATOR;
	}
	
	randomPasswordEnumeratorInitialize(&randomPasswordEnumerator,
					   &passwordGenerator,
					   enumeratorPassword,
					   RANDOM_PASSWORD_NUM_FACTOR * passwordGeneratorGetSize(&passwordGenerator));
	
	ret = RT_generate((passwordEnumerator_t *) &randomPasswordEnumerator,
			  &passwordGenerator,
			  enumeratorPassword,
			  generatorPassword,
			  hashFunc,
			  chainLength,
			  prefix,
			  entiresInHashTable,
			  bucketBlockLength,
			  TRUE, /* TODO: ? */
			  TRUE /* TODO: ? */
			  );

LBL_CLEANUP_GENERATOR:
	passwordGeneratorFinalize(&passwordGenerator);
LBL_CLEANUP_DICTIONARY:
	dictionaryFinalize(&dictionary);
LBL_ERROR:
	FREE(generatorPassword);
	FREE(enumeratorPassword);
	return ret;;
}

bool_t createRainbowTableUsingIni(const char * hashName,
				  const char * prefix,
				  const char * rule,
				  const char * iniFilename) {
	bool_t ret = FALSE;
	const char * keys[] = {	"dictionary_name",
				"rainbow_chain_length",
				"entires_in_hash_table",
				"bucket_block_length" };
	const char * values[sizeof(keys) / sizeof(*keys)] = {0};
	const uint_t numKeys = sizeof(keys) / sizeof(*keys);
	char * iniContent = NULL;
	
	/* Parse INI file */
	CHECK(verifyFileExists(iniFilename));
	iniContent = readEntireTextFile(iniFilename);
	CHECK(NULL != iniContent);
	CHECK(parseIni(iniContent, keys, values, numKeys));
	
	/* Invoke rainbow table creation */
	ret = createRainbowTable(hashName, prefix, rule, values[0], values[1], values[2], values[3]);

LBL_ERROR:
	FREE(iniContent);
	return ret;
}
