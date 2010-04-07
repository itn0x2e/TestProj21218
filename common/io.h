#ifndef __IO_H__
#define __IO_H__

#include <stdio.h>
#include "types.h"

/**
 * @param line
 *
 * @pre stream != NULL
 * @pre stream points to a valid FILE
 * @pre line != NULL
 * @pre line is allocated for at least MAX_LINE_LEN chars
 */
bool_t readLine(FILE * stream, char * line);

#endif /* __IO_H__ */