#include <stdio.h>
#include <stdio.h>

#include "types.h"
#include "util.h"
#include "constants.h"


char * calcHash(algorithmId algo, char * password)
{
	//! TODO: fill in
	return strdup(password);
}

bool writeUserAuth(FILE * fd, algorithmId algo, char * username, char * password) 
{
	bool ret = FALSE;
	char * hash = NULL;

	CHECK(fd != NULL);
	CHECK(username != NULL);
	CHECK(password != NULL);

	hash = calcHash(algo, password);

	fprintf(fd, "%s\t%s\n", username, hash);
	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;

LBL_CLEANUP:

	FREE(hash);

	return ret;
}

bool readUserAuth(FILE * fd, char ** username, char ** hash) 
{
	bool ret = FALSE;
	char line[MAX_LINE_LEN] = {0};

	CHECK(fd != NULL);
	CHECK(username != NULL);
	CHECK(hash != NULL);

	/* read line */
	fgets(line, sizeof(line), fd);

	/* temp setup */

	/* find token */
	if (NULL == strchr((char *) line, '\t')) {
		FAIL("bad file format");
	}

	/* temp setup */
	*hash = strchr((char *) line, '\t') + 1;

	*strchr(line, '\t') = '\0';
	*username = strdup(*username);	

	*hash = strdup(*hash);
	
	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;

LBL_CLEANUP:
	return ret;
}

void create_authentication(char * filename, algorithmId algo) 
{
	FILE * fd = fopen(filename, "a");
	
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
			FAIL("Error writing to file. quitting.\n");
		}
	}

	goto LBL_CLEANUP;

LBL_EOF:
	printf("\nbye.\n");
	goto LBL_CLEANUP;

LBL_ERROR:
	
LBL_CLEANUP:
	FCLOSE(fd);
}


void printUsage(void) 
{
	printf("usage: create_authentication [MD5|SHA1] <file>\n");
}


int main(int argc, char ** argv) 
{
	algorithmId algo = ALGO_INVALID;

	if (3 != argc) { 
		printUsage();
		return -1;
	}

	if (0 == strcasecmp(argv[1], "md5")) {
		algo = ALGO_MD5;
	} else if (0 == strcasecmp(argv[1], "sha1")) {
		algo = ALGO_SHA1;
	} else {
		printUsage();
		return -1;
	}

	create_authentication(argv[2], algo);
	
	return 0;
}
