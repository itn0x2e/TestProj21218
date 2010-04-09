#include "../common/constants.h"
#include "../common/misc.h"
#include "../common/ui.h"
#include "../common/utils.h"
#include "../DEHT/DEHT.h"
#include "../rainbow_table/rainbow_table.h"

int main(int argc, char** argv);
bool_t exhaustive_query(const char * prefix);
void commandLoop(RainbowTable_t * rainbowTable);
RainbowTable_t * initializeExaustiveTable(const char * prefix);
void finalizeExaustiveTable(RainbowTable_t * rainbowTable); /* TODO: this func should tolerate NULL as arg, and ignore it */
bool_t queryExaustiveTable(RainbowTable_t * rainbowTable, byte_t * hash, uint_t hashLen, char * password, ulong_t passwordLen); /* TODO: should receive size allocated for password? */

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
	RainbowTable_t * rainbowTable = NULL;
	
	CHECK(verifyDEHTExists(prefix));
	printf("1\n");
	CHECK(NULL != initializeExaustiveTable(prefix));
	printf("2\n");
	commandLoop(rainbowTable);
	printf("3\n");
	ret = TRUE;
	
LBL_ERROR:
	finalizeExaustiveTable(rainbowTable);
	return ret;
}

void commandLoop(RainbowTable_t * rainbowTable) {
	char line[MAX_LINE_LEN] = {0};
	byte_t hash[MAX_LINE_LEN] = {0}; /* The hash is never longer than its string representation */
	uint_t hashLen = 0;		
	char password[1000] = {0}; /* TODO: determine constant value */

	/* Treat the user's submitted requests until told to quit */
	for(;;) {
		if (!readPrompt(line) || (0 == strcmp(line, "quit"))) {
			return;
		}

		hashLen = (uint_t) hexa2binary(line, hash, sizeof(hash));
		if (-1 == hashLen) {
			fprintf(stderr, "Non hexa\n");
			continue;
		}
		
		if (queryExaustiveTable(rainbowTable, hash, hashLen, password, sizeof(password))) {
			printf("Try to login with password \"%s\"\n", password);
		} else {
			printf("Sorry but this hash doesn't appears in pre-processing\n");
		}
	}
}

RainbowTable_t * initializeExaustiveTable(const char * prefix) {
	return RT_open(NULL, NULL, prefix, FALSE, FALSE);
}

void finalizeExaustiveTable(RainbowTable_t * rainbowTable) {
	RT_close(rainbowTable);
}

bool_t queryExaustiveTable(RainbowTable_t * rainbowTable, byte_t * hash, uint_t hashLen, char * password, ulong_t passwordLen) {
	char s[1000];
	binary2hexa(hash, (int) hashLen, s, sizeof(s));
	printf("%s\n", s);
	
	return RT_query(rainbowTable, 
		hash, hashLen, password, passwordLen);
}