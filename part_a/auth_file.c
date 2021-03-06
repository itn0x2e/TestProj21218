#include <stdio.h>
#include "../common/constants.h"
#include "../common/io.h"
#include "../common/misc.h"
#include "../common/rand_utils.h"
#include "../common/types.h"
#include "auth_file.h"

static bool_t parseEntries(authFile_t * self, char * rawEntries);
static bool_t authFileAuthenticateWithSalt(const authFile_t * self, const char * username, const char * password);
static bool_t authFileAuthenticateWithoutSalt(const authFile_t * self, const char * username, const char * password);
static bool_t cryptHashWithSalt(BasicHashFunctionPtr cryptHashPtr, const char *passwd, unsigned char *outBuf, const byte_t * salt);

bool_t authFileInitialize(authFile_t * self, const char * filename, bool_t salty) {
	char * rawEntries = NULL;

	self->salty = salty;

	self->fileContent = readEntireTextFile(filename);
	if (NULL == self->fileContent) {
		return FALSE;
	}

	self->entries = NULL;

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
	if (self->salty) {
		return authFileAuthenticateWithSalt(self, username, password);
	}

	return authFileAuthenticateWithoutSalt(self, username, password);
}

bool_t writeUserAuth(FILE * fd, BasicHashFunctionPtr hashFunc, const char * username, const char * password, bool_t salty) {
	unsigned char hash[MAX_DIGEST_LEN];
	char asciiHash[2 * MAX_DIGEST_LEN + 1];
	byte_t salt[SALT_LEN];
	char asciiSalt[2 * SALT_LEN + 1];
	int hashSize;

	if (salty) {
		randomizeSalt(salt);

		hashSize = cryptHashWithSalt(hashFunc, password, hash, salt);
	} else {
		hashSize = cryptHash(hashFunc, password, hash);
	}

	binary2hexa(hash, hashSize, asciiHash, sizeof(asciiHash));

	if (salty) {
		binary2hexa(salt, SALT_LEN, asciiSalt, sizeof(asciiSalt));
		CHECK(0 <= fprintf(fd, "%s\t%s%s\n", username, asciiHash, asciiSalt));
	} else {
		CHECK(0 <= fprintf(fd, "%s\t%s\n", username, asciiHash));
	}

	return TRUE;

LBL_ERROR:
	perror("Error");
	return FALSE;
}

static bool_t parseEntries(authFile_t * self, char * rawEntries) {
	char * end = rawEntries + strlen(rawEntries);
	uint_t digestStrLength = getHashFunDigestLength(self->hashFunc) * 2;
	uint_t i;

	uint_t hexStrLength = digestStrLength;
	if (self->salty) {
		hexStrLength += SALT_LEN * 2;
	}

	for (i = 0; i < self->numEntries; ++i) {
		self->entries[i].username = rawEntries;

		rawEntries = strchr(rawEntries, '\t');
		CHECK(NULL != rawEntries);

		*rawEntries = '\0';
		++rawEntries;

		CHECK(	(end > rawEntries + hexStrLength) &&
				('\n' == rawEntries[hexStrLength]));

		if (self->salty) {
			rawEntries[hexStrLength] = '\0';
			CHECK(-1 != hexa2binary(rawEntries + digestStrLength, self->entries[i].salt, sizeof(self->entries[i].salt)));
		}

		rawEntries[digestStrLength] = '\0';
		CHECK(-1 != hexa2binary(rawEntries, self->entries[i].hash, sizeof(self->entries[i].hash)));

		rawEntries += hexStrLength + 1;
	}

	return TRUE;

LBL_ERROR:
	return FALSE;
}

static bool_t authFileAuthenticateWithSalt(const authFile_t * self, const char * username, const char * password) {
	uint_t i = 0;

	for (i = 0; i < self->numEntries; ++i) {
		if (0 == strcmp(username, self->entries[i].username)) {
			byte_t hashedSaltyPassword[MAX_DIGEST_LEN] = {0};
			int hashLen = cryptHashWithSalt(self->hashFunc, password, hashedSaltyPassword, self->entries[i].salt);

			if (0 == memcmp(hashedSaltyPassword, self->entries[i].hash, hashLen)) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

static bool_t authFileAuthenticateWithoutSalt(const authFile_t * self, const char * username, const char * password) {
	byte_t hashedPassword[MAX_DIGEST_LEN] = {0};
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

static bool_t cryptHashWithSalt(BasicHashFunctionPtr cryptHashPtr, const char *passwd, unsigned char *outBuf, const byte_t * salt) {
	bool_t ret = FALSE;
	uint_t passwdLen = strlen(passwd);
	uint_t saltedPasswordLen = passwdLen + SALT_LEN;
	char * saltedPassword = (char *) malloc(sizeof(char) * saltedPasswordLen);
	if (NULL == saltedPassword) {
		perror("Error");
		return FALSE;
	}
	memcpy(saltedPassword, passwd, passwdLen);
	memcpy(saltedPassword + passwdLen, salt, SALT_LEN);

	ret = cryptHashPtr((const unsigned char *) saltedPassword, saltedPasswordLen ,outBuf);

	free(saltedPassword);

	return ret;
}
