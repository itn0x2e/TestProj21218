#include <stdio.h>
#include <string.h>
#include "../common/types.h"
#include "../common/ui.h"
#include "../common/utils.h"
#include "../common/misc.h"
#include "../common/constants.h"
#include "auth_file.h"
#include "create_authentication_common.h"

static bool_t commandLoop(FILE * file, BasicHashFunctionPtr hashFunc, bool_t salty);

bool_t create_authentication(char * filename, const char * hashFuncName, bool_t salty) {
	bool_t ret = FALSE;
	BasicHashFunctionPtr hashFunc = NULL;
	FILE * file = NULL;

	/* Validate arguments */
	if (!parseHashFunName(&hashFunc, hashFuncName) || !verifyFileNotExist(filename)) {
		return FALSE;
	}

	/* Create an empty authentication file for writing */
	file = fopen(filename, "w");
	if (NULL == file) {
		perror(filename);
		return FALSE;
	}

	/* Write the name of the hash function into the file */
	if ((1 != fwrite(hashFuncName, strlen(hashFuncName), 1, file)) || (1 != fwrite("\n", 1, 1, file))) {
		perror(filename);
		FCLOSE(file);
		return FALSE;
	}

	/* Treat user commands */
	ret = commandLoop(file, hashFunc, salty);

	FCLOSE(file);
	return ret;
}

static bool_t commandLoop(FILE * file, BasicHashFunctionPtr hashFunc, bool_t salty) {
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

		if (!writeUserAuth(file, hashFunc, user, password, salty)) {
			return FALSE;
		}
	}
}
