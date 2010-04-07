#ifndef __AUTH_FILE_H__
#define __AUTH_FILE_H__

#include "../common/misc.h"
#include "../common/types.h"

typedef struct authFileEntry_s {
	char * username;

	/* hash is in binary form, not a hex string */
	byte_t * hash;
} authFileEntry_t;

typedef struct authFile_s {
	BasicHashFunctionPtr hashFunc;
	authFileEntry_t * entries;
	uint_t numEntries;
} authFile_t;

bool_t authFileInitialize(authFile_t * self, const char * filename);
void authFileFinalize(authFile_t * self);
bool_t authFileAuthenticate(const authFile_t * authFile, const char * username, const char * password);

bool_t writeUserAuth(FILE * fd, BasicHashFunctionPtr hashFunc, const char * username, const char * password);
bool_t readUserAuth(FILE * fd, authFileEntry_t * res);

#endif /* __AUTH_FILE_H__ */
