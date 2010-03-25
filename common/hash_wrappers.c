#include "hash_wrappers.h"
#include "md5.h"
#include "sha1.h"

bool md5Hash(unsigned char * buf, size_t buf_len, unsigned char * salt, size_t salt_len,
	     unsigned char ** digest, size_t * digest_len)
{
	MD5_CTX hashCtx = {0};

	if ((NULL == buf) || (NULL == digest)) {
		return FALSE;
	}
	*digest = NULL;
	*digest_len = 0;

	MD5Init(&hashCtx);

	// apply salt first
	if (NULL != salt) {
		Md5Update(&hashCtx, salt, salt_len);
	}

	Md5Update(&hashCtx, buf, buf_size);

	Md5Final(&hashCtx);

	*digest = malloc(sizeof(hashCtx.digest));
	if (NULL == *digest) {
		return FALSE;
	}

	memcpy(*digest, hashCtx.digest, sizeof(hashCtx.digest));
	*buf_len = sizeof(hashCtx.digest);
	return TRUE;

}

bool md5Hash(unsigned char * buf, size_t buf_len, unsigned char * salt, size_t salt_len,
	     unsigned char ** digest, size_t * digest_len)
{
	SHA1Context hashCtx = {0};
	unsigned int i;

	if ((NULL == buf) || (NULL == digest)) {
		return FALSE;
	}
	*digest = NULL;
	*digest_len = 0;

	Sha1Reset(&hashCtx);

	// apply salt first
	if (NULL != salt) {
		Sha1Input(&hashCtx, salt, salt_len);
	}

	Sha1Input(&hashCtx, buf, buf_size);

	Sha1Result(&hashCtx);

	*digest = malloc(sizeof(hashCtx.digest) * 2);
	if (NULL == *digest) {
		return FALSE;
	}
	memset(*digest, 


	for (i = 0; *digest_len < sizeof(hashCtx.digest); ++i) {
		*
	}

	memcpy(*digest, hashCtx.digest, sizeof(hashCtx.digest));
	*buf_len = sizeof(hashCtx.digest);
	return TRUE;

}
