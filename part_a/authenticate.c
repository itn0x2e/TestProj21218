#include <stdio.h>
#include <string.h>

#include "../common/types.h"
#include "../common/util.h"
#include "../common/misc.h"
#include "../common/constants.h"
#include "auth_file.h"



bool_t authenticate(char * filename) 
{
	bool_t ret = FALSE;
	authFile_t authFile = {0};

	CHECK(NULL != filename);

	/* Load auth file to memory */
	if (!readAuthFile(filename, &authFile)) {
		FAIL("unable to read auth file");
	}
	

	/* Test the user's submitted passwords until told to quit */
	for(;;) {
		char line[MAX_LINE_LEN] = {0};
		char * user = NULL;
		char * pass = NULL;
		
		printf(">>> ");
		if (NULL == fgets(line, sizeof(line), stdin)) {
			goto LBL_EOF;
		}

		/* trim string */
		if ('\n' == line[strlen(line) - 1]) { 
			line[strlen(line) - 1] = '\0';
		}

		if (0 == strcmp(line, "quit")) {
			goto LBL_EOF;
		}

		user = line;
		/* verify input */
		if (NULL == strchr((char *) line, '\t')) {
			printf("bad request\n");
			continue;
		}

		pass = strchr(line, '\t') + 1;
		*(pass - 1) = '\0';
		
		if (authenticateAgainstAuthFile(&authFile, user, pass)) {
			printf("Approved.\n");
		}
		else {
			printf("Denied.\n");
		}
	}

	goto LBL_CLEANUP;

LBL_EOF:
	ret = TRUE;

	printf("\nbye.\n");
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	
LBL_CLEANUP:
	freeAuthFile(&authFile);

	return ret;
}


void printUsage(void) 
{
	printf("usage: authenticate <file>\n");
}


int main(int argc, char ** argv) 
{
	if (2 != argc) { 
		printUsage();
		return -1;
	}

	/* return 0 if create_authentication returned successfully */
	return (TRUE != authenticate(argv[1]));
}
