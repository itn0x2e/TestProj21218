#include <stdio.h>
#include <string.h>

#include "../common/types.h"
#include "../common/utils.h"
#include "../common/misc.h"
#include "../common/constants.h"
#include "auth_file.h"



bool_t create_authentication(char * filename, algorithmId_t algo) 
{
	bool_t ret = FALSE;

	char * algoName = NULL;
	FILE * fd = NULL;
	
	CHECK(NULL != filename);

	fd = fopen(filename, "w");
	if (NULL == fd) {
		FAIL("fopen hash file");
	}

	switch (algo) {
	case ALGO_MD5:
		algoName = "MD5\n";
		break;
	case ALGO_SHA1:
		algoName = "SHA1\n";
		break;
	default:
		FAIL("invalid algorithm id");
	}
	if (1 != fwrite(algoName, strlen(algoName), 1, fd)) {
		FAIL("fwrite hash algorith id to hash file");
	}

	
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
		
		if (!writeUserAuth(fd, algo, user, pass)) {
			FAIL("Error writing new auth credentials to file");
		}
	}

	goto LBL_CLEANUP;

LBL_EOF:
	printf("\nbye.\n");

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	
LBL_CLEANUP:
	FCLOSE(fd);

	return ret;
}


void printUsage(void) 
{
	printf("usage: create_authentication [MD5|SHA1] <file>\n");
}


int main(int argc, char ** argv) 
{
	algorithmId_t algo = ALGO_INVALID;

	if (3 != argc) { 
		printUsage();
		return -1;
	}

	if (0 == strcmp(argv[1], "MD5")) {
		algo = ALGO_MD5;
	} else if (0 == strcmp(argv[1], "SHA1")) {
		algo = ALGO_SHA1;
	} else {
		printUsage();
		return 1;
	}

	/* return 0 if create_authentication returned successfully */
	return (TRUE != create_authentication(argv[2], algo));
}
