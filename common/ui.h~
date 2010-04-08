#ifndef __UI_H__
#define __UI_H__

#include "misc.h"
#include "types.h"
#include "../password/dictionary.h"

bool_t validateRule(const char * rule);
bool_t validateDEHTPrefix(const char * prefix);
bool_t validateFileNotExist(const char * filename);

bool_t parseIni(char * content, const char ** keys, const char ** values, uint_t numKeys);
bool_t parseHashFunName(BasicHashFunctionPtr * hashFunc, const char * name);
bool_t readDictionaryFromFile(dictionary_t * dictionary, const char * filename);

/**
 * @param line
 *
 * @pre line != NULL
 * @pre line is allocated for at least MAX_LINE_LEN chars
 */
bool_t readPrompt(char * line);


#endif /* __UI_H__ */