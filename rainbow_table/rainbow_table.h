#ifndef __RAINBOW_TABLE_H__
#define __RAINBOW_TABLE_H__

#include "../common/types.h"
#include "../common/misc.h"
#include "../password/password_enumerator.h"
#include "../password/password_generator.h"
#include "../DEHT/DEHT.h"

typedef LONG_INDEX_PROJ RainbowSeed_t;

/* data stored inside DEHT's user bytes */
typedef struct RainbowTableConfig_s {
	ulong_t chainLength;
	RainbowSeed_t seeds[1];
} RainbowTableConfig_t;

/* instance */
typedef struct RainbowTable_s {
	DEHT * hashTable;
	RainbowTableConfig_t * config;
	BasicHashFunctionPtr hashFunc;
	const passwordGenerator_t * passGenerator;
	char * password;
	ulong_t passwordLength;
} RainbowTable_t;


ulong_t getConfigSize(ulong_t chainLength);

/**
* Build a Rainbow Table according to the specified parameters. This is only a creation interface
* for queries, see RT_open() & RT_query().
* Function desc: This function will generate a new Rainbow Table as per the user's request and
*		 save it to disk. The table can then be opened using RT_open() and queried using
*		 RT_query() (and closed with RT_close())
*
* @param passwordEnumerator - a password enumerator over the password space. Expected to be a random
*			      password enumerator over the password range, of about 10 * the actual
*			      range (to increase likelihood of a hit for queries)
* @param passwordGenerator - a password generator (random access to the entire password range)
* @param enumeratorPassword - the buffer associated with the password enumerator
* @param generatorPassword - the buffer associated with the password generator
* @param hashFunc - valid pointer the one of the hash function (SHA-1 / MD-5)
* @param rainbowChainLen - length of the rainbow-chains
* @param hashTableFilePrefix - file prefix for DEHT hash table files. files are generated
*			       for hashTableFilePrefix.key and hashTableFilePrefix.data
* @param nHashTableEntries - number of buckets in the hash table. This may require some 
*			     tuning to get decent performance.
*			  performance.
* @param nPairsPerBlock - number of key<->data pairs to store in each disk block in the 
*			  hash table. This may require some tuning to get decent performance.
*
* @ret TRUE on success, FALSE otherwise.
*
*/
bool_t RT_generate(passwordEnumerator_t * passwordEnumerator,
		   const passwordGenerator_t * passwordGenerator,
		   char * enumeratorPassword,
		   char * generatorPassword,
		   BasicHashFunctionPtr hashFunc,
		   ulong_t rainbowChainLen,
		   const char * hashTableFilePrefix,
		   ulong_t nHashTableEntries,
		   ulong_t nPairsPerBlock);

/**
* Open an existing rainbow table for querying
* Function desc: This function will open a valid existing Rainbow Table previously created using
*		 RT_generate().  The returned object can then be queried using RT_query() 
*		 (and closed with RT_close())
*
* @param passwordGenerator - a password generator (random access to the entire password range)
* @param password - The buffer to use for returning query results and for internal
*		    password processing.
* @param passwordLength - maximum length for the buffer pointed to by 'password', in bytes.
* @param hashTableFilePrefix - file prefix for DEHT hash table files. files are generated
*			       for hashTableFilePrefix.key and hashTableFilePrefix.data
* @param enableFirstBlockCache - If true, we will attempt to use DEHT's first block ptr
*				 cache to speed up queries.
*
* @ret A pointer to a valid instance if successful, NULL otherwise.
*      This instance MUST BE freed by RT_close, and not just by 'free'!
*/
RainbowTable_t * RT_open(const passwordGenerator_t * passGenerator,
			 char * password,
			 ulong_t passwordLength,
			 const char * hashTableFilePrefix,
			 bool_t enableFirstBlockCache);

/**
* Close an existing rainbow table and free all associated resources
* Function desc: This function will close a valid Rainbow Table previously opened using
*		 RT_open().  
*
* @param self - pointer to a valid Rainbow Table instance
*
* @ret None.
*/
void RT_close(RainbowTable_t * self);


/**
* Query the rainbow table for a hash, and attempt to return the original password for said hash.
* Function desc: Using the algorithm detailed in the project spec, this function attempts to
*		 find the original password for the given hash
*
* @param self - valid pointer to a rainbow table instance
* @param hash - hash to be 'reversed'
* @param hashLen - length of hash
* @param found - out parameter - was the algorithm able to deduce the original password for
*		 the requested hash.
*
* @ret TRUE if no errors were encountered, FALSE otherwise.
*      If the query was successful, the recovered password is stored in the buffer originally
*      passed to RT_open.
*/
bool_t RT_query(RainbowTable_t * self, const byte_t * hash, ulong_t hashLen, bool_t * found);


/**
* Print important information regarding the rainbow table, and verify the correctness of its
* content (as per the project spec for "text_export_rainbow_table")
* Function desc: This function dumps to the two files the following information:
*		 In seedsAndPasswordsFd:
*		 -Seeds used to generate the chains
*		 -Passwords used to generate the chains
*		 In chainsFd:
*		 -Every stage in each of the chains stored in the hash table (tab delimited).
*		 -an error message for each chain that yielded an incorrect final hash
*
* @param seedsAndPasswordsFd - file pointer in write mode, to dump seeds and password info
*			       to.
* @param chainsFd - file pointer in write mode, to dump chaing info to.
* @param passGenerator - password generator to use. Must be identical to the one
*			 used to generate the rainbow table
* @param generatorPassword - password buffer for passGenerator
* @param passwordMaxLen - maximum length of the password buffer
* @param hashTableFilePrefix - prefix for DEHT files to open. The files must exist and must
*			       be the result of a successful call to RT_generate
*
* @ret TRUE if no errors were encountered, FALSE otherwise.
*/
bool_t RT_print(FILE * seedsAndPasswordsFd,
		FILE * chainsFd,

		const passwordGenerator_t * passGenerator,
		char * generatorPassword,
		ulong_t passwordMaxLen,
		const char * hashTableFilePrefix);

#endif /* __RAINBOW_TABLE_H__ */
