#ifndef __AUTH_FILE_H__
#define __AUTH_FILE_H__

#include "../common/misc.h"
#include "../common/rand_utils.h"
#include "../common/types.h"

typedef struct authFileEntry_s {
	const char * username;

	/* hash is in binary form, not a hex string */
	byte_t hash[MAX_DIGEST_LEN];

	byte_t salt[SALT_LEN];
} authFileEntry_t;

typedef struct authFile_s {
	char * fileContent;
	BasicHashFunctionPtr hashFunc;
	authFileEntry_t * entries;
	uint_t numEntries;
	bool_t salty;
} authFile_t;

bool_t authFileInitialize(authFile_t * self, const char * filename, bool_t salty);
void authFileFinalize(authFile_t * self);
bool_t authFileAuthenticate(const authFile_t * authFile, const char * username, const char * password);

bool_t writeUserAuth(FILE * fd, BasicHashFunctionPtr hashFunc, const char * username, const char * password, bool_t salty);

#endif /* __AUTH_FILE_H__ */
