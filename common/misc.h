/************************************************************************/
/* File misc.h, some nice interfaces along with partial implementations */
/************************************************************************/

#ifndef _MISC_H_
#define _MISC_H_

#include <string.h>
#include "utils.h"
#include "types.h"

#define SYMBOL2NIBBLE(symbol) ((((symbol) >= 'a') * 9) + ((symbol) & 0xf))
#define NIBBLE2SYMBOL(nibble) (('0' + (((nibble) >= 10) * ('a' - '0')) + ((nibble) % 10)))

/*************************************************************************/
/* typedef BasicHashFunctionPtr:                                         */
/* A cryptographic hash function performs many bitic operation on a      */
/* variable size binary array and returns a fixed size one:              */
/* Define its type by BasicHashFunctionPtr                               */
/* It returns number of bytes actually used in output buffer (32 in SHA1)*/
/* As sizes are fixed, relatively short and well known, assume output    */
/* buffer is long enough to contain these all.                           */
/*************************************************************************/
typedef int (*BasicHashFunctionPtr)(const unsigned char *,int,unsigned char *);
/* Arguments are: const unsigned char *inbuf, i.e. binary input*/
/* int inputLength , i.e. its length in byte, as it may vary   */
/* unsigned char *outBuf output buffer to fulfill. Assume long enough*/

#define MD5_OUTPUT_LENGTH_IN_BYTES    (16)
int MD5BasicHash ( const unsigned char *in,int len, unsigned char *outBuf); 

#define SHA1_OUTPUT_LENGTH_IN_BYTES   (32)
int SHA1BasicHash ( const unsigned char *in,int len, unsigned char *outBuf); 

#define HASH_MAX_SIZE (MAX(SHA1_OUTPUT_LENGTH_IN_BYTES, MD5_OUTPUT_LENGTH_IN_BYTES))


/*************************************************************************/
/* Function cryptHash do it on an ascii string.                          */
/* Inputs:                                                               */
/* A function pointer to crytographic hash (e.g. MD5BasicHash)           */
/* A null terminated string of ascii password we want to hash            */
/* An output binary buffer (to fullfil without null termination)         */
/* Output: number bytes fulfilled in output (e.g. 20 for MD5)            */
/* usage like cryptHash ( MD5BasicHash , passwd , outbufWith20Bytes );   */
/*************************************************************************/
int cryptHash ( BasicHashFunctionPtr cryptHashPtr, const char *passwd, unsigned char *outBuf );
/* See implementation in misc_partial.c*/ 

/*************************************************************************/
/* Functions hexa2binary and binary2hexa are parallel to atoi and itoa   */
/* but use little-endian and longer binary size. So strings are read or  */
/* written in a reverse order than what we used to in Math.              */
/* Advantage of little-endian that it is very easy to implement, easier  */
/*   to read, and more compatible with the way things arranged in mem.   */
/*************************************************************************/
/*************************************************************************/
/* Function hexa2binary transforms each pair of hexa characters into     */
/*   a single output byte.                                               */
/* Inputs:                                                               */
/*  A null terminated ascii string (strIn).                              */
/*  A buffer to write output into (outBuf) not null terminated.          */
/*  Length of buffer (outMaxLen) not to exceed - avoid memory violation  */
/* Output: numer of bytes fulfilled in outBuf, or -1 if                  */
/*   either wanted to exceed memory limit, or string was not hexa        */
/* Examples:                                                             */
/*   hexa2binary("5b3",(unsigned char *)&integerVariable,4) puts         */
/*      949 in integerVariable and return 2                              */
/*   hexa2binary("820c5",(unsigned char *)&shortVar,2) puts 23554,       */
/*      returns 2 (as we limit 2 bytes for filling)                      */
/*   hexa2binary("89y",(unsigned char *)&shortVar,2) returns -1, not hexa*/
/*************************************************************************/
int hexa2binary(const char *strIn, unsigned char *outBuf, int outMaxLen);
/*************************************************************************/
/* Function binary2hexa transforms each input byte into two hexa chars   */
/* Inputs:                                                               */
/*  A binary buffer (bufIn) and its length in bytes (lengthIn).          */
/*  An output string to fulfil(outStroutStr) and memory limit (outMaxLen)*/
/* Output: numer of bytes fulfilled in outStr, or -1 if wanted to exceed */
/* Note: outStr is null terminated even in case of failour.              */
/*************************************************************************/
int binary2hexa(const unsigned char *bufIn, int lengthIn,
				char *outStr, int outMaxLen);

/*************************************************************************/
/* In real rainbow-tables number of possible passwords to examine is     */
/* typically larger than 2^32 thus index of such password shall be 64bit */
/*************************************************************************/
/*************************************************************************/
/* type definition of LONG_INDEX_PROJ is a 64bit index can password space*/
/*************************************************************************/
#define LONG_INDEX_PROJ unsigned long

/*************************************************************************/
/* Function pseudo_random_generator_proj replaces rand for 64bit         */
/* It also randomizes better (i.e. results are more close to white noise)*/
/* Input is an index you use. same index causes same answer.             */
/* Output is always 63bit (i.e. top bit=0) to avoid sign issue bugs      */
/* Note: this MD5 based implementation is the simplest, thought not      */
/*   efficient. But during project, few calls for this function.         */
/*   Implementation gives you a hint how do program the slightly more    */
/*   complex object "pseudo_random_generator_proj"                       */
/*************************************************************************/
LONG_INDEX_PROJ pseudo_random_generator_proj(int step);
/*See implementation in misc_partial.c . You are allowed to change it !  */

/*************************************************************************/
/* Function pseudo_random_function, is those Rj in defined in background */
/* Inputs:                                                               */
/*   A binary buffer (i.e. pointer "x" and size "inputLength").          */
/*   A seed - source of randomness for this function                     */
/* Output:   A 64bit index (top bit always 0).                           */
/* Note: for a fixed seed, this can be considered as a "random" function */
/* activating on x. To get a familiy o such functions, use severl seeds. */
/* Function is purely deterministic, same results for same args          */
/* It is allowed to change interface a little if you want more efficient */
/*   implementation. Just make it as generic as this one.                */
/*************************************************************************/
LONG_INDEX_PROJ pseudo_random_function(const unsigned char *x,int inputLength,LONG_INDEX_PROJ seed);

BasicHashFunctionPtr getHashFunFromName(const char * name);

const char * getNameFromHashFun(BasicHashFunctionPtr hashFunc);

unsigned int getHashFunDigestLength(BasicHashFunctionPtr hashFunc);



int MD5SeededHash(byte_t * outBuf, 
		   const byte_t * seed, ulong_t seedLen, 
		   const byte_t * data, ulong_t dataLen);
int SHA1SeededHash(byte_t * outBuf, 
		   const byte_t * seed, ulong_t seedLen, 
		   const byte_t * data, ulong_t dataLen);

int miniHash(byte_t * output, ulong_t outputLen,
		const byte_t * seed, ulong_t seedLen,
		const byte_t * data, int dataLen);

#endif
/**********************  EOF (misc.h) *********************/
