#ifndef __UI_H__
#define __UI_H__

#include <stdio.h>
#include "constants.h"
#include "types.h"

/**
 * @param line
 *
 * @pre line != NULL
 * @pre line is allocated for at least MAX_LINE_LEN chars
 */
bool_t readPrompt(char * line);

#endif /* __UI_H__ */