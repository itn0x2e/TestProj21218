#include <stdio.h>
#include <string.h>
#include "../common/types.h"
#include "../common/ui.h"
#include "../common/utils.h"
#include "../common/misc.h"
#include "../common/constants.h"
#include "auth_file.h"

int main(int argc, char ** argv);
bool_t create_authentication(char * filename, const char * hashFuncName);
bool_t commandLoop(FILE * file, BasicHashFunctionPtr hashFunc);

int main(int argc, char ** argv) {
	if (3 != argc) {
		fprintf(stderr, "Error: Usage create_authentication <hash function name> <filename to create>\n");
		return 1;
	}

	/* return 0 if create_authentication returned successfully, otherwise 1 */
	if (create_authentication(argv[2], argv[1])) {
		return 0;
	}

	return 1;
}

bool_t create_authentication(char * filename, const char * hashFuncName) {
	/* TODO: check whether the file already exist? */
	bool_t ret = FALSE;
	BasicHashFunctionPtr hashFunc = getHashFunFromName(hashFuncName);
	FILE * file = NULL;

	if (NULL == hashFunc) {
		fprintf(stderr, "Error: Hash \"%s\" is not supported\n", hashFuncName);
		return FALSE;
	}

	ASSERT(NULL != filename);

	file = fopen(filename, "w");
	if (NULL == file) {
		/* TODO: print error msg? FAIL("fopen hash file"); */
		return FALSE;
	}

	if ((1 != fwrite(hashFuncName, strlen(hashFuncName), 1, file)) ||
			(1 != fwrite("\n", 1, 1, file))) {
		/* TODO: reconsider error msg */
		FCLOSE(file);
		return FALSE;
	}
	
	ret = commandLoop(file, hashFunc);

	FCLOSE(file);

	return ret;
}

bool_t commandLoop(FILE * file, BasicHashFunctionPtr hashFunc) {
	char line[MAX_LINE_LEN] = {0};
	const char * user = NULL;
	const char * password = NULL;
	char* separator = NULL;

	for(;;) {
		if (!readPrompt(line) || (0 == strcmp(line, "quit"))) {
			return TRUE;
		}

		separator = strchr(line, '\t');

		/* Verify input */
		if (NULL == separator) {
			fprintf(stderr, "Error: Commands are either \"quit\" or <user name>tab<password>.\n");
			continue;
		}

		/* The user name is whatever proceeds the separator */
		user = line;
		*separator = '\0';

		/* Since the password may contain ' ', we simply consider it to be
		 * everything that follows the separator */
		password = separator + 1;

		if (!writeUserAuth(file, hashFunc, user, password)) {
			/* TODO: consider some error msg */
			return FALSE;
		}
	}
}
