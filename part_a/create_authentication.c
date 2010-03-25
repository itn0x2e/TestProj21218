#include <stdio.h>
#include <string.h>

#include "../common/types.h"
#include "../common/util.h"
#include "../common/misc.h"
#include "../common/constants.h"


int calcHash(algorithmId algo, char * password, unsigned char * out)
{
	switch(algo) {
	case ALGO_MD5:
		return MD5BasicHash(password, strlen(password), out);
	case ALGO_SHA1:
		return SHA1BasicHash(password, strlen(password), out);		
	default:
		return 0;
	}
}

bool_t writeUserAuth(FILE * fd, algorithmId algo, char * username, char * password) 
{
	unsigned char hash[HASH_MAX_SIZE];
	char asciiHash[2*HASH_MAX_SIZE + 1];

	bool_t ret = FALSE;
	int hashSize = 0;

	CHECK(fd != NULL);
	CHECK(username != NULL);
	CHECK(password != NULL);

	hashSize = calcHash(algo, password, hash);
	printf("hashSize=%d\n", hashSize);
	binary2hexa(hash, hashSize, asciiHash, sizeof(asciiHash));
	printf("strlen(asciiHash)=%lu\n", strlen(asciiHash));
	fprintf(fd, "%s\t%s\n", username, asciiHash);

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;

LBL_CLEANUP:

	return ret;
}

bool_t readUserAuth(FILE * fd, char ** username, char ** hash) 
{
	bool_t ret = FALSE;
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
