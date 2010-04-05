#include <stdio.h>
#include "auth_file.h"
#include "../common/types.h"
#include "../common/constants.h"
#include "../common/misc.h"


int calcHash(algorithmId_t algo, char * password, unsigned char * out)
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


bool_t writeUserAuth(FILE * fd, algorithmId_t algo, char * username, char * password) 
{
	unsigned char hash[HASH_MAX_SIZE];
	char asciiHash[2*HASH_MAX_SIZE + 1];

	bool_t ret = FALSE;
	int hashSize = 0;

	CHECK(fd != NULL);
	CHECK(username != NULL);
	CHECK(password != NULL);

	hashSize = calcHash(algo, password, hash);
	binary2hexa(hash, hashSize, asciiHash, sizeof(asciiHash));
	fprintf(fd, "%s\t%s\n", username, asciiHash);

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;

LBL_CLEANUP:

	return ret;
}

bool_t readUserAuth(FILE * fd, accountInfo_t * res) 
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
	res->user = strdup(line);

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


bool_t readAuthFile(char * filename, authFile_t * res)
{
	bool_t ret = FALSE;

	char algoName[MAX_LINE_LEN] = {0};

	int filePtr = 0;
	unsigned int identityCount = 0;
	accountInfo_t tempCredentials = {0};
	accountInfo_t * credentialsArr = NULL;

	FILE * fd = NULL;

	CHECK(NULL != filename);
	CHECK(NULL != res);

	res->algo = ALGO_INVALID;
	res->entryCount = 0;
	res->entries = NULL;

	/* open file */
	fd = fopen(filename, "r");
	if (NULL == fd) {
		FAIL("fopen hash file");
	}

	/* read algo id */
	if (NULL == fgets(algoName, sizeof(algoName), fd)) {
		FAIL("fread algorithm id");
	}
	/* trim trailing '\n' */
	if ('\n' == algoName[strlen(algoName)-1]) {
		algoName[strlen(algoName)-1] = '\0';
	}

	/* parse algo id */
	if (0 == strcasecmp(algoName, "md5")) {
		res->algo = ALGO_MD5;
	} else if (0 == strcasecmp(algoName, "sha1")) {
		res->algo = ALGO_SHA1;
	} else {
		FAIL("invalid algorithm ID");
	}

	/* save old file ptr */
	filePtr = ftell(fd);

	/* count entries in the file */
	for(identityCount = 0; readUserAuth(fd, &tempCredentials); ++identityCount) {
		FREE(tempCredentials.user);
		FREE(tempCredentials.hash);
	}
	res->entryCount = identityCount;


	/* restore old file ptr */
	fseek(fd, filePtr, SEEK_SET);

	/* alloc array for credential info */
	res->entries = (accountInfo_t *) malloc(sizeof(accountInfo_t) * identityCount);
	if (NULL == res->entries) {
		FAIL("malloc credentials array failed");
	}

	/* scan the records, again, this time keeping them in memory */
	for(identityCount = 0; readUserAuth(fd, (res->entries) + identityCount); ++identityCount) {
		/* Do nothing here (everything is done in the loop construct) */
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	
LBL_CLEANUP:
	FCLOSE(fd);

	return ret;
}

void freeAuthFile(authFile_t * authFile) 
{
	unsigned int i = 0;

	if (NULL == authFile) {
		return;
	}

	for(i = 0; i < authFile->entryCount; ++i) {
		FREE(authFile->entries[i].user);
		FREE(authFile->entries[i].hash);
	}

	FREE(authFile->entries);
}

bool_t authenticateAgainstAuthFile(authFile_t * authFile, char * user, char * password)
{
	bool_t ret = FALSE;
	unsigned int i = 0;
	unsigned char hashedPassword[HASH_MAX_SIZE] = {0};
	int hashLen = 0;

	CHECK(NULL != authFile);
	CHECK(NULL != user);
	CHECK(NULL != password);

	for (i = 0; i < authFile->entryCount; ++i) {
		if (0 != strcmp(user, authFile->entries[i].user)) {
			continue;
		}

		/* calc hash on password */
		hashLen = calcHash(authFile->algo, password, hashedPassword);
		if (0 == hashLen) {
			FAIL("Internal error while computing hash");
		}
		
		/* compare results */
		if (0 == memcmp(hashedPassword, authFile->entries[i].hash, hashLen)) {
			ret = TRUE;
		}
		else {
			ret = FALSE;
		}

		goto LBL_CLEANUP;
		
	}

LBL_ERROR:
	ret = FALSE;
	
LBL_CLEANUP:
	return ret;

}



