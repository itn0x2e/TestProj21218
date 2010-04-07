#ifndef __AUTH_FILE_H__
#define __AUTH_FILE_H__

#include "../common/misc.h"
#include "../common/types.h"

typedef struct accountInfo_s {
	char * user;

	/* hash is in binary form, not a hex string */
	byte_t * hash;
} accountInfo_t;

typedef struct authFile_s {
	BasicHashFunctionPtr hashFunc;
	unsigned int entryCount;
	accountInfo_t * entries;
} authFile_t;

bool_t writeUserAuth(FILE * fd, BasicHashFunctionPtr hashFunc, const char * username, const char * password);
bool_t readUserAuth(FILE * fd, accountInfo_t * res);

bool_t authFileInitialize(authFile_t * self, const char * filename);
void authFileFinalize(authFile_t * self);
bool_t authFileAuthenticate(const authFile_t * authFile, const char * username, const char * password);

#endif /* __AUTH_FILE_H__ */
