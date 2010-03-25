#ifndef __HASH_WRAPPERS_H__
#define __HASH_WRAPPERS_H__

#include "types.h"

bool md5Hash(unsigned char * buf, size_t size, unsigned char * salt, size_t salt_len,
	     unsigned char * digest, size_t * digest_len);
bool sha1Hash(unsigned char * buf, size_t size, unsigned char * salt, size_t salt_len,
	     unsigned char * digest, size_t * digest_len);

#endif // __HASH_WRAPPERS_H__
