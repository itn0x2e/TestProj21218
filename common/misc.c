#include <string.h>
#include <stdio.h>
#include "constants.h"
#include "md5.h"
#include "misc.h"
#include "sha1.h"
#include "constants.h"

#define SYMBOL2NIBBLE(symbol) ((((symbol) >= 'a') * 9) + ((symbol) & 0xf))
#define NIBBLE2SYMBOL(nibble) (('0' + (((nibble) >= 10) * ('a' - '0')) + ((nibble) % 10)))

LONG_INDEX_PROJ pseudo_random_generator_proj(int step)
{
	LONG_INDEX_PROJ hashedBuffer[MD5_OUTPUT_LENGTH_IN_BYTES/sizeof(LONG_INDEX_PROJ)];
	int tmp2hashBuf[2];
	tmp2hashBuf[0] = step; /*make sure step affects result*/
	tmp2hashBuf[1] = 0xbadf00d; /*make your program different from other*/
	/*Now, hash 'step' into a pseudo-random 16Bytes buffer:*/
	MD5BasicHash ( (unsigned char *)tmp2hashBuf,
		2*sizeof(int),(unsigned char *)hashedBuffer);
	/*now re-interpret first 8 bytes in hashedBuffer as result, kill sign bit*/
	return ((hashedBuffer[0])&0x7fffffffffffffff); 
}

int cryptHash(BasicHashFunctionPtr cryptHashPtr, const char *passwd, unsigned char *outBuf )
{
	return cryptHashPtr ((const unsigned char *) passwd, strlen(passwd) , outBuf) ; 
}

int MD5BasicHash(const unsigned char *in, int len, unsigned char *outBuf)
{
	/* when you want to compute MD5, first, declere the next struct */
	/*! TODO: restore !*/
	MD5_CTX mdContext;/*! = {0};!*/

	if ((NULL == in) || (NULL == outBuf)) {
		return 0;
	}

	/* then, init it before the first use */
	MD5Init (&mdContext);

	/* compute your string's hash using the next to calls */
	MD5Update (&mdContext, (unsigned char *)in, len);
	MD5Final (&mdContext);

	memcpy(outBuf,mdContext.digest, sizeof(mdContext.digest));

	return sizeof(mdContext.digest);
}

int SHA1BasicHash ( const unsigned char *in,int len, unsigned char *outBuf)
{
	/* when you want to compute SH!, first, declere the next struct */
	/*! TODO: restore !*/
	SHA1Context hashCtx;/*! = {0};!*/

	if ((NULL == in) || (NULL == outBuf)) {
		return 0;
	}

	/* then, init it before the first use */
	SHA1Reset(&hashCtx);

	/* compute your string's hash using the next to calls */
	SHA1Input(&hashCtx, (unsigned char *) in, len);
	SHA1Result(&hashCtx);

	/* hashCtx.Message_Digest is an array of u16's. Since we want to output an array of u8's,
	   anything we do will be sort of a hack, so this one will do */
	memcpy(outBuf, (unsigned char *) hashCtx.Message_Digest, sizeof(hashCtx.Message_Digest));

	return sizeof(hashCtx.Message_Digest);
}

int hexa2binary(const char *strIn, unsigned char *outBuf, int outMaxLen) {
	int lengthIn = strlen(strIn);
	int i;
	
	if (((lengthIn + 1) / 2) > outMaxLen) {
		return -1;
	}
	
	for (i =0; i < lengthIn; ++i) {
		if (('0' > strIn[i]) || (('9' < strIn[i]) && ('a' > strIn[i])) || ('f' < strIn[i])) {
			return -1;
		}
		
		outBuf[i / 2] &= 0xf0;
		outBuf[i / 2] |= SYMBOL2NIBBLE(strIn[i]);
		outBuf[i / 2] <<= (4 * ((i + 1) % 2));
	}
	
	return ((lengthIn + 1) / 2);
}

int binary2hexa(const unsigned char *bufIn, int lengthIn,
				char *outStr, int outMaxLen) {
	int i;
	
	if ((2 * lengthIn + 1) > outMaxLen) {
		/* outStr is null terminated even in case of failure */
		*outStr = '\0';
		return -1;
	}
	
	for (i =0; i < lengthIn; ++i) {
		outStr[2 * i] = (char) NIBBLE2SYMBOL((bufIn[i] >> 4));
		outStr[(2 * i) + 1] = (char) NIBBLE2SYMBOL(bufIn[i] & 0xf);
	}
	
	outStr[2 * lengthIn] = '\0';
	return (2 * lengthIn);
}

BasicHashFunctionPtr getHashFunFromName(const char * name) {
	if (0 == strcmp(name, "MD5")) {
		return MD5BasicHash;
	}
	if (0 == strcmp(name, "SHA1")) {
		return SHA1BasicHash;
	}
	return NULL;
}

unsigned int getHashFunDigestLength(BasicHashFunctionPtr hashFunc) {
	if (MD5BasicHash == hashFunc) {
		return MD5_DIGEST_LEN;
	}
	if (SHA1BasicHash == hashFunc) {
		return SHA1_DIGEST_LEN;
	}
	return 0;
}

/*
const char * getNameFromHashFun(BasicHashFunctionPtr hashFunc) {
	switch(hashFunc) {
	case MD5BasicHash:
		return "MD5";
	case SHA1BasicHash:
		return "SHA1";
	}
	return NULL;
}
*/


