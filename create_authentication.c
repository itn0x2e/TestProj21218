#include <stdio.h>
#include <stdio.h>

#include "types.h"
#include "util.h"
#include "constants.h"


bool writeUserAuth(FILE * fd, char * username, char * password) 
{
	bool ret = FALSE;

	CHECK(fd != NULL);
	CHECK(username != NULL);
	CHECK(password != NULL);

	fprintf(fd, "%s\t%s\n", username, password);
	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;

LBL_CLEANUP:
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
	*hash = strchr((char *) line, "\t") + 1;

	*strchr(line, "\t") = '\0';
	*username = strdup(*username);	

	*hash = strdup(*hash);
	
	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;

LBL_CLEANUP:
	return ret;
}

void create_authentication(char * filename) 
{
	FILE * fd = fopen(filename, "a");
	
	for(;;) {
		char line[MAX_LINE_LEN] = {0};
		char * user = NULL;
		char * pass = NULL;

		fgets(stdin, line, sizeof(line));
		user = line;
		/* verify input */
		if (NULL == strchr(line, '\t')) {
			printf("bad request\n");
			continue;
		}

		pass = strchr(line, '\t') + 1;
		*(pass - 1) = '\0';
		
		if (!writeUserAuth(fd, user, pass)) {
			FAIL("Error writing to file. quitting.\n");
		}
	}

LBL_ERROR:
	
LBL_CLEANUP:
	if (NULL != fd) {
		fclose(fd);
		fd = NULL;
	}
}


int main(int argc, char ** argv) 
{
	writeAuth(NULL, NULL, NULL);
	

}
