#ifndef __UI_H__
#define __UI_H__

#include "types.h"

bool_t validateRule(const char * rule);

/**
 * @param line
 *
 * @pre line != NULL
 * @pre line is allocated for at least MAX_LINE_LEN chars
 */
bool_t readPrompt(char * line);

bool_t parseIni(char * content, const char ** keys, const char ** values, uint_t numKeys);

#endif /* __UI_H__ */