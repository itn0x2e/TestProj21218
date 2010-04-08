#include <stdio.h>
#include <string.h>
#include "../common/ui.h"
#include "../password/dictionary.h"
#include "../password/all_password_enumerator.h"
#include "../password/password_enumerator.h"
#include "../password/password_generator.h"
#include "../password/random_password_enumerator.h"

#define PAIRS_PER_BLOCK (7)
#define SIGNATURE_LENGTH_PER_KEY (8)
#define TABLE_OF_POINTERS_ENTRIES (65536) /* 2^16 */
/* TODO: what about the 16-bit hash function???????????? [Use 65536 (2^16) entries in the table-of-pointers (i.e. 16-bit hash function).] */

bool_t exhaustive_table_generator(const char * rule,
				  const char * dictionaryFilename,
				  const char * hashName,
				  const char * prefix,
				  const char * flag);

void printUsage();

void initializePasswordEnumerator(passwordEnumerator_t * enumerator,
				  const passwordGenerator_t * generator,
				  char * password,
				  bool_t all,
				  ulong_t n);

bool_t parseFlag(const char * flag, bool_t * all, ulong_t * n);

bool_t generateExaustiveTable(passwordEnumerator_t * enumerator,
			      char * password,
			      BasicHashFunctionPtr hashFunc,
			      const char * prefix);

int main(int argc, char** argv) {
	if (6 != argc) {
		printUsage();
		return 1;
	}
	
	if (exhaustive_table_generator(argv[1], argv[2], argv[3], argv[4], argv[5])) {
		return 0;
	}
	
	return 1;
}

bool_t exhaustive_table_generator(const char * rule,
				  const char * dictionaryFilename,
				  const char * hashName,
				  const char * prefix,
				  const char * flag) {
	bool_t ret = FALSE;
	BasicHashFunctionPtr hashFunc;
	bool_t all;
	ulong_t n;
	dictionary_t dictionary;
	passwordGenerator_t passwordGenerator;
	char * password = NULL;
	passwordEnumerator_t * passwordEnumerator = NULL;
	
	CHECK(validateRule(rule));
	CHECK(parseHashFunName(&hashFunc, hashName));
	CHECK(verifyDEHTNotExist(prefix));
	CHECK(parseFlag(flag, &all, &n));
	CHECK(readDictionaryFromFile(&dictionary, dictionaryFilename));
	
	if (!passwordGeneratorInitialize(&passwordGenerator, rule, &dictionary)) {
		goto LBL_CLEANUP_DICTIONARY;
	}
	
	password = (char *) malloc((passwordGeneratorGetMaxLength(&passwordGenerator) + 1) * sizeof(char));
	if (NULL == password) {
		PERROR();
		goto LBL_CLEANUP_GENERATOR;
	}
	
	passwordEnumerator = (passwordEnumerator_t *) malloc(all ? sizeof(allPasswordEnumerator_t) :
								   sizeof(randomPasswordEnumerator_t));
	if (NULL == passwordEnumerator) {
		PERROR();
		goto LBL_CLEANUP_GENERATOR;
	}
	
	initializePasswordEnumerator(passwordEnumerator, &passwordGenerator, password, all, n);
	
	ret = generateExaustiveTable(passwordEnumerator, password, hashFunc, prefix);

LBL_CLEANUP_GENERATOR:
	passwordGeneratorFinalize(&passwordGenerator);
LBL_CLEANUP_DICTIONARY:
	dictionaryFinalize(&dictionary);
LBL_ERROR:
	FREE(password);
	FREE(passwordEnumerator);
	return ret;;
}

void printUsage() {
	fprintf(stderr, "Error: Usage exhaustive_table_generator <rule> <dictionary> "
			"<hash> <filenames prefix> <\"all\" or number of random password>\n");
}

void initializePasswordEnumerator(passwordEnumerator_t * enumerator,
				  const passwordGenerator_t * generator,
				  char * password,
				  bool_t all,
				  ulong_t n) {
	if (all) {
		allPasswordEnumeratorInitialize((allPasswordEnumerator_t *) enumerator, generator, password);
	} else {
		randomPasswordEnumeratorInitialize((randomPasswordEnumerator_t *) enumerator, generator, password, n);
	}
}

bool_t parseFlag(const char * flag, bool_t * all, ulong_t * n) {
	if (0 == strcmp(flag, "all")) {
		*all = TRUE;
		/* TODO: document that the value of *n is undefined */
		return TRUE;
	}
	
	if (1 == sscanf(flag, "%lu", n)) {
		*all = FALSE;
		return TRUE;
	}
	printUsage();
	return FALSE;
}

bool_t generateExaustiveTable(passwordEnumerator_t * enumerator,
			      char * password,
			      BasicHashFunctionPtr hashFunc,
			      const char * prefix) {
	/* TODO: dummy */
	while(passwordEnumeratorCalculateNextPassword(enumerator)) printf("%s\n", password);
	return TRUE;
}