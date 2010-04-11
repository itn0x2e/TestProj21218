#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include "../common/types.h"

/**
 * Represents case of letters.
 */
typedef enum letterCase_e {
	LOWER_CASE = 0,
	UPPER_CASE = 1
} letterCase_t;

/**
 * A pair of strings.
 */
typedef char * stringPair_t[2];

/**
 * A dictionary.
 *
 * Enables random-access to entries.
 * Entries get be retrieved in either lower or upper case.
 *
 * @note	In order to accelerate queries, entries are held as pairs of
 * 		variations - one string for each letter case possibility.
 */
typedef struct dictionary_s {
	/* The number of dictionary entries */
	uint_t numEntries;

	/* The number of characters in the longest dictionary word, not including
	 * the terminating null */
	uint_t maxEntryLength;

	/* A copy of the raw data - lower cased followed by upper cased */
	char * rawDataLowerUpper;

	/* The dictionary entries as pairs of strings - one for each letter case
	 * possibility */
	stringPair_t * entries;
} dictionary_t;

/**
 * Initializes the dictionary from its raw representation.
 *
 * @param self		The dictionary to initialize
 * @param rawDict	The raw dictionary
 *
 * @pre		self != NULL
 * @pre		rawDict != NULL
 *
 * @return	TRUE if initialized successfully, otherwise FALSE.
 *
 * @note	If FALSE is returned, finalization should not be invoked.
 */
bool_t dictionaryInitialize(dictionary_t * self, const char * rawDict);

/**
 * A finalization function.
 *
 * @param self	The dictionary to finalize
 * @pre		self != NULL
 * @pre		self is initialized.
 */
void dictionaryFinalize(dictionary_t * self);

/**
 * @param self	A dictionary
 * @pre		self != NULL
 * @pre		self is initialized.
 *
 * @return 	The number of entries in the dictionary
 */
ulong_t dictionaryGetSize(const dictionary_t * self);

/**
 * @param self	A dictionary
 * @pre		self != NULL
 * @pre		self is initialized.
 *
 * @return	The number of characters in the longest dictionary word, not including
 * 		the terminating null
 */
ulong_t dictionaryGetMaxLength(const dictionary_t * self);

/**
 * Retrieves a dictionary word.
 *
 * @param self		A dictionary
 * @param index		The index inside the dictionary
 * @param letterCase	The requested case for the word
 *
 * @pre		self != NULL
 * @pre		self is initialized.
 * @pre		index < dictionaryGetSize(self)
 */
const char * dictionaryGetEntry(const dictionary_t * self,
				ulong_t index,
				letterCase_t letterCase);

#endif /* __DICTIONARY_H__ */
