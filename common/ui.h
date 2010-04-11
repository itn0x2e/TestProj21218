#ifndef __UI_H__
#define __UI_H__

#include "misc.h"
#include "types.h"
#include "../password/dictionary.h"

/**
 * Validates the syntax of a rule and possibly outputs an error message.
 *
 * @param rule		A string representation of a rule.
 * @return whether the rule has a valid syntax.
 *
 * @pre rule != NULL
 */
bool_t validateRule(const char * rule);

/**
 * Verifies that DEHT files exist and possibly outputs an error message.
 *
 * @param prefix	The file prefix
 * @return whether the DEHT files exist
 *
 * @pre prefix != NULL
 */
bool_t verifyDEHTExists(const char * prefix);

/**
 * Verifies that DEHT files do not exist and possibly outputs an error message.
 *
 * @param prefix	The file prefix
 * @return whether the DEHT files do not exist
 *
 * @pre prefix != NULL
 */
bool_t verifyDEHTNotExist(const char * prefix);

/**
 * Verifies that a file exists and possibly outputs an error message.
 *
 * @param filename	The file name
 * @return whether the file exists
 *
 * @pre prefix != NULL
 */
bool_t verifyFileExists(const char * filename);

/**
 * Verifies that a file does not exists and possibly outputs an error message.
 *
 * @param filename	The file name
 * @return whether the file does not exist
 *
 * @pre prefix != NULL
 */
bool_t verifyFileNotExist(const char * filename);

/**
 * Parses the content of an INI file and possibly outputs an error message.
 *
 * @param content	The content
 * @param keys		An array of key strings
 * @param values	An array for the values. NULL for values not found
 * @param numKeys	The number of keys.
 *
 * @pre	The number of values in the array equals to numKeys
 *
 * @return TRUE upon success, FALSE upon failure.
 */
bool_t parseIni(char * content, const char ** keys, const char ** values, uint_t numKeys);

/**
 * Parses the number from an INI file and possibly outputs an error message.
 *
 * @param str	A string representation of the number
 * @param num	An output param for it
 *
 * @return TRUE upon success, FALSE upon failure.
 */
bool_t parseIniNum(const char * str, ulong_t * num);

/**
 * Outputs the keys and values taken from an INI file, for keys which appear in it.
 *
 * @param keys		An array of key strings
 * @param values	An array of the values. NULL for values not found
 * @param numKeysThe number of keys.
 *
 * @pre	The number of values in the array equals to numKeys
 */
void printIni(const char ** keys, const char ** values, uint_t numKeys);

/**
 * Parses a name of an hash function and possibly outputs an error message.
 *
 * @param name		A string representation of the number
 * @param hashFunc	An output param for the function
 *
 * @return TRUE upon success, FALSE upon failure.
 */
bool_t parseHashFunName(BasicHashFunctionPtr * hashFunc, const char * name);

/**
 * Parses a dictionary file and possibly outputs an error message.
 *
 * @param filename	The name of the dictionary file.
 * @param dictionary	A output param for the dictionary allocated 
 *
 * @return TRUE upon success, FALSE upon failure.
 */
bool_t readDictionaryFromFile(dictionary_t * dictionary, const char * filename);

/**
 * Shows a prompt to the user and reads a line.
 *
 * @param line A output param for the line read. The trailing '\n' is trimmed.
 *
 * @pre line != NULL
 * @pre line is allocated for at least MAX_LINE_LEN chars
 *
 * @return TRUE upon success, FALSE upon failure.
 */
bool_t readPrompt(char * line);

#endif /* __UI_H__ */
