#include "misc.h"
#include "md5.h"

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

int cryptHash ( BasicHashFunctionPtr cryptHashPtr, const char *passwd, unsigned char *outBuf )
{
	return cryptHashPtr ( passwd, strlen(passwd) , outBuf) ; 
}

int MD5BasicHash ( const unsigned char *in,int len, unsigned char *outBuf)
{
  /* when you want to compute MD5, first, declere the next struct */
  MD5_CTX mdContext;
  /* then, init it before the first use */
  MD5Init (&mdContext);

  /* compute your string's hash using the next to calls */
  MD5Update (&mdContext, (unsigned char *)in, len);
  MD5Final (&mdContext);

  memcpy(outBuf,mdContext.digest,MD5_OUTPUT_LENGTH_IN_BYTES);
  return MD5_OUTPUT_LENGTH_IN_BYTES;
}

