#include "../common/constants.h"
#include "../common/misc.h"
#include "../common/ui.h"
#include "../common/utils.h"
#include "../DEHT/DEHT.h"

int main(int argc, char** argv);
bool_t exhaustive_query(const char * prefix);
void commandLoop(DEHT * deht);
DEHT * initializeExaustiveTable(const char * prefix);
void finalizeExaustiveTable(DEHT * deht); /* TODO: this func should tolerate NULL as arg, and ignore it */
bool_t queryExaustiveTable(DEHT * deht, const byte_t * hash, int hashSize, char * password); /* TODO: should receive size allocated for password? */

int main(int argc, char** argv) {
	if (2 != argc) {
		fprintf(stderr, "Error: Usage exhaustive_query <filenames prefix>\n");
		return 1;
	}
	
	if (exhaustive_query(argv[1])) {
		return 0;
	}
	
	return 1;
}

bool_t exhaustive_query(const char * prefix) {
	bool_t  ret = FALSE;
	DEHT * deht = NULL;
	
	CHECK(verifyDEHTExists(prefix));
	
	CHECK(NULL != initializeExaustiveTable(prefix));
	
	commandLoop(deht);
	
	ret = TRUE;
	
LBL_ERROR:
	finalizeExaustiveTable(deht);
	return ret;
}

void commandLoop(DEHT * deht) {
	char line[MAX_LINE_LEN] = {0};
	byte_t hash[MAX_LINE_LEN] = {0}; /* The hash is never longer than its string representation */
	int hashSize = 0;
	char password[1000] = {0}; /* TODO: determine constant value */

	/* Treat the user's submitted requests until told to quit */
	for(;;) {
		if (!readPrompt(line) || (0 == strcmp(line, "quit"))) {
			return;
		}

		hashSize = hexa2binary(line, hash, sizeof(hash));
		if (-1 == hashSize) {
			fprintf(stderr, "Non hexa\n");
			continue;
		}
		
		if (queryExaustiveTable(deht, hash, hashSize, password)) {
			printf("Try to login with password \"%s\"\n", password);
		} else {
			printf("Sorry but this hash doesn't appears in pre-processing\n");
		}
	}
}

DEHT * initializeExaustiveTable(const char * prefix) {
	/* TODO: dummy */
	return (DEHT * ) 1;
}

void finalizeExaustiveTable(DEHT * deht) {
	/* TODO: dummy */
}

bool_t queryExaustiveTable(DEHT * deht, const byte_t * hash, int hashSize, char * password) {
	/* TODO: dummy */
	char s[1000];
	binary2hexa(hash, hashSize, s, sizeof(s));
	printf("%s\n", s);
	return TRUE;
}