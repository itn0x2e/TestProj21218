

#ifndef __AUTH_FILE_H__
#define __AUTH_FILE_H__

#include "../common/types.h"
#include "../common/misc.h"
#include "../common/util.h"

typedef struct accountInfo_s {
	char * user;

	/* hash is in binary form, not a hex string */
	unsigned char * hash;
} accountInfo_t;

typedef struct authFile_s {
	algorithmId_t algo;
	unsigned int entryCount;
	accountInfo_t * entries;
} authFile_t;

bool_t writeUserAuth(FILE * fd, algorithmId_t algo, char * username, char * password);
bool_t readUserAuth(FILE * fd, accountInfo_t * res);

bool_t readAuthFile(char * filename, authFile_t * res);
void freeAuthFile(authFile_t * authFile);

bool_t authenticateAgainstAuthFile(authFile_t * authFile, char * user, char * password);

#endif /* __AUTH_FILE_H__ */
