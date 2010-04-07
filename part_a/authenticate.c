#include <stdio.h>
#include <string.h>

#include "../common/constants.h"
#include "../common/misc.h"
#include "../common/types.h"
#include "../common/ui.h"
#include "../common/utils.h"
#include "auth_file.h"

int main(int argc, char ** argv);
bool_t authenticate(char * filename);
void commandLoop(authFile_t * authFile);

int main(int argc, char ** argv) {
	if (2 != argc) { 
		fprintf(stderr, "Error: Usage authenticate <authentication table text file>\n");
		return 1;
	}

	/* return 0 if authenticate returned successfully, otherwise 1 */
	if (authenticate(argv[1])) {
		return 0;
	}
	
	return 1;
}

bool_t authenticate(char * filename) {
	authFile_t authFile = {0};

	/* Load auth file to memory */
	if (!authFileInitialize(&authFile, filename)) { /* TODO: should we invoke finalize in this case? */
		/* TODO: reconsider error message */
		fprintf(stderr, "unable to read auth file");
		return FALSE;
	}
	
	commandLoop(&authFile);

	authFileFinalize(&authFile);

	return TRUE;
}

void commandLoop(authFile_t * authFile) {
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
