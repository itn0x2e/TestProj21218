#include <string.h>
#include <stdio.h>
#include "misc.h"
#include "md5.h"
#include "sha1.h"

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
	MD5_CTX mdContext;/* = {0};*/

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
	SHA1Context hashCtx;/* = {0};*/

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

int binary2hexa(const unsigned char *bufIn, int lengthIn,
				char *outStr, int outMaxLen)
{
	int i = 0;
	char tempOut[2 + 1] = {0};
	memset(outStr, 0, outMaxLen);

	for (i = 0; (((i/2) < lengthIn) && (i < (outMaxLen - 1))); i += 2) {
		/*snprintf(tempOut, sizeof(tempOut), "%02X", bufIn[i/2]);*/
		sprintf(tempOut, "%02X", bufIn[i/2]);
		outStr[i] = tempOut[0];
		outStr[i+1] = tempOut[1];
	}

	/* Terminate the string for sure */
	outStr[outMaxLen - 1] = 0x00;

	return MIN(2 * lengthIn + 1, outMaxLen);
}

int hexa2binary(const char *strIn, unsigned char *outBuf, int outMaxLen)
{
	int i = 0;
	char tempByte[3] = {0};
	unsigned int temp = 0;

	for (i = 0; ( ((2 * i) < strlen(strIn)) && (i < (outMaxLen - 1))); ++i) {
		tempByte[0] = strIn[2*i];
		tempByte[1] = strIn[2*i + 1];
		tempByte[3] = 0x00;


		sscanf(tempByte, "%02X", &temp);
		outBuf[i] = temp;
	}

	return MIN(strlen(strIn) / 2, outMaxLen);
}
