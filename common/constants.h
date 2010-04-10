#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include "md5.h"
#include "sha1.h"
#include "utils.h"

#define MAX_USERNAME_LEN (80)
#define MAX_PASSWORD_LEN (80)

#define MAX_LINE_LEN (256)

#define SHA1_DIGEST_LEN (sizeof(((SHA1Context *) 0)->Message_Digest))
#define MD5_DIGEST_LEN (sizeof(((MD5_CTX *) 0)->digest))
#define MAX_DIGEST_LEN (MAX(SHA1_DIGEST_LEN, MD5_DIGEST_LEN))

#define MAX_FILE_MODE_LEN (10)

#define INT_MAX_POSITIVE_VALUE (0x7fffffff)

#endif /* __CONSTANTS_H__ */
