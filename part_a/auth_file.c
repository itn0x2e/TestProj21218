#include <stdio.h>
#include "../common/constants.h"
#include "../common/io.h"
#include "../common/misc.h"
#include "../common/types.h"
#include "auth_file.h"


/* Needed on some machines (Ubuntu 9.04 with some patch config) since strdup's 
 * definition was broken in some versions of gnu libc */
extern char *strdup (__const char *__s);  /* TODO: tmp */

bool_t authFileInitialize(authFile_t * self, const char * filename)
{
	bool_t ret = FALSE;

	char hashFuncName[MAX_LINE_LEN] = {0};

	long int filePtr = 0;
	unsigned int identityCount = 0;
	authFileEntry_t tempCredentials = {0};

	FILE * file = NULL;

	CHECK(NULL != filename);

	self->hashFunc = NULL;
	self->numEntries = 0;
	self->entries = NULL;

	/* open file */
	file = fopen(filename, "rb"); /* TODO: should be "rb" */
	if (NULL == file) {
		/* TODO: reconsider error msg */
		FAIL("fopen hash file");
	}

	if (!readLine(file, hashFuncName)) {
		/* TODO: reconsider error msg */
		FAIL("fread algorithm id");
	}

	self->hashFunc = getHashFunFromName(hashFuncName);

	if (NULL == self->hashFunc) {
		/* TODO: reconsider error msg */
		FAIL("invalid algorithm ID");
	}

	/* save old file ptr */
	filePtr = ftell(file);

	/* count entries in the file */
	/* TODO: shouldn't we check the return value of readUserAuth? */
	for(identityCount = 0; readUserAuth(file, &tempCredentials); ++identityCount) {
		FREE(tempCredentials.username);
		FREE(tempCredentials.hash);
	}
	self->numEntries = identityCount;


	/* restore old file ptr */
	fseek(file, filePtr, SEEK_SET);

	/* alloc array for credential info */
	self->entries = (authFileEntry_t *) malloc(sizeof(authFileEntry_t) * identityCount);
	if (NULL == self->entries) {
		FAIL("malloc credentials array failed");
	}

	/* scan the records, again, this time keeping them in memory */
	/* TODO: shouldn't we check the return value of readUserAuth? */
	for(identityCount = 0; readUserAuth(file, (self->entries) + identityCount); ++identityCount) {
		/* Do nothing here (everything is done in the loop construct) */
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	
LBL_CLEANUP:
	FCLOSE(file);

	return ret;
}

void authFileFinalize(authFile_t * self) {
	unsigned int i = 0;

	/* TODO: is this done if Initizlize fails? */
	for(i = 0; i < self->numEntries; ++i) {
		FREE(self->entries[i].username);
		FREE(self->entries[i].hash);
	}

	FREE(self->entries);
}

bool_t authFileAuthenticate(const authFile_t * self, const char * username, const char * password) {
	unsigned char hashedPassword[HASH_MAX_SIZE] = {0};
	int hashLen = cryptHash(self->hashFunc, password, hashedPassword);
	uint_t i = 0;

	printf("%d\n", hashLen); /* TODO: just for debugging */

	for (i = 0; i < self->numEntries; ++i) {
		if (0 == strcmp(username, self->entries[i].username)) {
			/* compare results */
			if (0 == memcmp(hashedPassword, self->entries[i].hash, hashLen)) {
				return TRUE;
			}
		}
	}

	return FALSE;
}


bool_t writeUserAuth(FILE * fd, BasicHashFunctionPtr hashFunc, const char * username, const char * password)
{
	unsigned char hash[HASH_MAX_SIZE];
	char asciiHash[2*HASH_MAX_SIZE + 1];

	bool_t ret = FALSE;
	int hashSize = 0;

	CHECK(fd != NULL);
	CHECK(username != NULL);
	CHECK(password != NULL);

	hashSize = hashFunc((unsigned char *) password, strlen(password), hash);
	binary2hexa(hash, hashSize, asciiHash, sizeof(asciiHash));
	fprintf(fd, "%s\t%s\n", username, asciiHash);

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;

LBL_CLEANUP:

	return ret;
}

bool_t readUserAuth(FILE * fd, authFileEntry_t * res)
{
	bool_t ret = FALSE;
	char line[MAX_LINE_LEN] = {0};
	char * tempHash = NULL;

	CHECK(fd != NULL);
	CHECK(res != NULL);

	/* read line */
	if (NULL == fgets(line, sizeof(line), fd)) {
		goto LBL_ERROR;
	}

	/* temp setup */

	/* find token */
	if (NULL == strchr((char *) line, '\t')) {
		FAIL("bad file format");
	}

	/* temp setup */
	tempHash = strchr((char *) line, '\t') + 1;

	*strchr(line, '\t') = '\0';
	res->username = strdup(line);

	res->hash = malloc(strlen(tempHash) / 2 + 1);
	if (NULL == res->hash) {
		FAIL("malloc bin hash result");
	}

	if ((strlen(tempHash) / 2) != hexa2binary(tempHash, res->hash, (strlen(tempHash) / 2) + 1)) {
		FAIL("binary2hexa failed");
	}


	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;

LBL_CLEANUP:
	return ret;
}





