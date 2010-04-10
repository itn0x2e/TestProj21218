#include "../common/constants.h"
#include "../common/misc.h"
#include "../common/ui.h"
#include "../common/utils.h"
#include "../DEHT/DEHT.h"
#include "../rainbow_table/rainbow_table.h"

int main(int argc, char** argv);
bool_t exhaustive_query(const char * prefix);
void commandLoop(RainbowTable_t * rainbowTable, char * password);
RainbowTable_t * initializeExaustiveTable(const char * prefix, char * password, ulong_t passwordLen);
void finalizeExaustiveTable(RainbowTable_t * rainbowTable); /* TODO: this func should tolerate NULL as arg, and ignore it */
bool_t queryExaustiveTable(RainbowTable_t * self, const byte_t * hash, ulong_t hashLen, bool_t * found); /* TODO: should receive size allocated for password? */

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
	RainbowTable_t * rainbowTable = NULL;
	char password[1000]; /* TODO: determine some fixed size */
	
	if (verifyDEHTExists(prefix)) {
		rainbowTable = initializeExaustiveTable(prefix, password, sizeof(password) - 1);
		if (NULL != rainbowTable) {
			commandLoop(rainbowTable, password);
			
			finalizeExaustiveTable(rainbowTable);
			return TRUE;	
		}
	}
	
	return FALSE;
}

void commandLoop(RainbowTable_t * rainbowTable, char * password) {
	char line[MAX_LINE_LEN] = {0};
	byte_t hash[MAX_LINE_LEN] = {0}; /* The hash is never longer than its string representation */
	uint_t hashLen = 0;		

	/* Treat the user's submitted requests until told to quit */
	for(;;) {
		bool_t found = FALSE;

		if (!readPrompt(line) || (0 == strcmp(line, "quit"))) {
			return;
		}

		hashLen = (uint_t) hexa2binary(line, hash, sizeof(hash));
		if (-1 == hashLen) {
			fprintf(stderr, "Non hexa\n");
			continue;
		}
		
		if (!queryExaustiveTable(rainbowTable, hash, hashLen, &found)) {
			/* An error has occured and a message has already been printed */
			return;
		}
		if (found) {
			printf("Try to login with password \"%s\"\n", password);
		} else {
			printf("Sorry but this hash doesn't appears in pre-processing\n");
		}
	}
}

RainbowTable_t * initializeExaustiveTable(const char * prefix, char * password, ulong_t passwordLen) {
	return RT_open(NULL, password, passwordLen, prefix, FALSE, FALSE);
}

void finalizeExaustiveTable(RainbowTable_t * rainbowTable) {
	RT_close(rainbowTable);
}

bool_t queryExaustiveTable(RainbowTable_t * self, const byte_t * hash, ulong_t hashLen, bool_t * found) {
	char s[1000];
	binary2hexa(hash, (int) hashLen, s, sizeof(s));
	printf("queryExaustiveTable	%s\n", s);
	
	return RT_query(self, hash, hashLen, found);
}
