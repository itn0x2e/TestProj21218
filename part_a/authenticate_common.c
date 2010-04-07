#include <stdio.h>
#include <string.h>
#include "../common/constants.h"
#include "../common/misc.h"
#include "../common/types.h"
#include "../common/ui.h"
#include "../common/utils.h"
#include "auth_file.h"
#include "authenticate_common.h"

static void commandLoop(authFile_t * authFile);

bool_t authenticate(char * filename, bool_t salty) {
	authFile_t authFile = {0};

	/* Load auth file to memory */
	if (!authFileInitialize(&authFile, filename, salty)) {
		return FALSE;
	}
	
	commandLoop(&authFile);

	authFileFinalize(&authFile);

	return TRUE;
}

static void commandLoop(authFile_t * authFile) {
	char line[MAX_LINE_LEN] = {0};
	const char * user = NULL;
	const char * password = NULL;
	char* separator = NULL;

	/* Test the user's submitted passwords until told to quit */
	for(;;) {
		if (!readPrompt(line) || (0 == strcmp(line, "quit"))) {
			return;
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

		if (authFileAuthenticate(authFile, user, password)) {
			printf("Approved.\n");
		} else {
			printf("Denied.\n");
		}
	}
}
