#include <stdio.h>
#include "../common/constants.h"
#include "../common/io.h"
#include "../common/misc.h"
#include "../common/types.h"
#include "auth_file.h"


/* Needed on some machines (Ubuntu 9.04 with some patch config) since strdup's 
 * definition was broken in some versions of gnu libc */
extern char *strdup (__const char *__s);  /* TODO: tmp */

static bool_t parseEntries(authFile_t * self, char * rawEntries);

bool_t authFileInitialize(authFile_t * self, const char * filename) {
	char * rawEntries = NULL;

	self->fileContent = readEntireTextFile(filename);
	if (NULL == self->fileContent) {
		return FALSE;
	}

	self->entries = NULL;
	self->numEntries = 0;

	rawEntries = strchr(self->fileContent, '\n');
	CHECK(NULL != rawEntries);
	*rawEntries = '\0';
	++rawEntries;

	self->hashFunc = getHashFunFromName(self->fileContent);
	CHECK(NULL != self->hashFunc);

	self->numEntries = countNewlines(rawEntries);

	/* alloc array for credential info */
	self->entries = (authFileEntry_t *) malloc(sizeof(authFileEntry_t) * self->numEntries);

	if (NULL == self->entries) {
		perror(filename);
		goto LBL_CLEANUP;
	}

	CHECK(parseEntries(self, rawEntries));

	return TRUE;

LBL_ERROR:
	fprintf(stderr, "%s: Authentication file corruption\n", filename);

LBL_CLEANUP:
	authFileFinalize(self);
	return FALSE;
}

void authFileFinalize(authFile_t * self) {
	FREE(self->fileContent);
	FREE(self->entries);
}

bool_t authFileAuthenticate(const authFile_t * self, const char * username, const char * password) {
	unsigned char hashedPassword[MAX_DIGEST_LEN] = {0};
	int hashLen = cryptHash(self->hashFunc, password, hashedPassword);
	uint_t i = 0;

	for (i = 0; i < self->numEntries; ++i) {
		if ((0 == strcmp(username, self->entries[i].username)) &&
			(0 == memcmp(hashedPassword, self->entries[i].hash, hashLen))) {

			return TRUE;
		}
	}

	return FALSE;
}

bool_t writeUserAuth(FILE * fd, BasicHashFunctionPtr hashFunc, const char * username, const char * password) {
	unsigned char hash[MAX_DIGEST_LEN];
	char asciiHash[2 * MAX_DIGEST_LEN + 1];
	int hashSize = cryptHash(hashFunc, password, hash);

	binary2hexa(hash, hashSize, asciiHash, sizeof(asciiHash));

	if (0 > fprintf(fd, "%s\t%s\n", username, asciiHash)) {
		perror("ERROR");
		return FALSE;
	}

	return TRUE;
}

static bool_t parseEntries(authFile_t * self, char * rawEntries) {
	char * end = rawEntries + strlen(rawEntries);
	uint_t expectedHexStrLength = getHashFunDigestLength(self->hashFunc) * 2;
	uint_t i;

	for (i = 0; i < self->numEntries; ++i) {
		self->entries[i].username = rawEntries;

		rawEntries = strchr(rawEntries, '\t');
		CHECK(NULL != rawEntries);

		*rawEntries = '\0';
		++rawEntries;

		CHECK(	(end > rawEntries + expectedHexStrLength) &&
				('\n' == rawEntries[expectedHexStrLength]));

		rawEntries[expectedHexStrLength] = '\0';
		CHECK(-1 != hexa2binary(rawEntries, self->entries[i].hash, sizeof(self->entries[i].hash)));

		rawEntries += expectedHexStrLength + 1;
	}

	return TRUE;

LBL_ERROR:
	return FALSE;
}
