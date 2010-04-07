#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include "md5.h"
#include "sha1.h"
#include "utils.h"

#define MAX_USERNAME_LEN (20)
#define MAX_PASSWORD_LEN (32)

#define MAX_LINE_LEN (MAX_USERNAME_LEN + MAX_PASSWORD_LEN + 2)

#define SALT_LEN (6)

#define SHA1_DIGEST_LEN (sizeof(((SHA1Context *) 0)->Message_Digest))
#define MD5_DIGEST_LEN (sizeof(((MD5_CTX *) 0)->digest))
#define MAX_DIGEST_LEN (MAX(SHA1_DIGEST_LEN, MD5_DIGEST_LEN))

#endif /* __CONSTANTS_H__ */
