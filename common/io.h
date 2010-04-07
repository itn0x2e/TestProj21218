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

byte_t * readEntireBinaryFile(const char * filename, ulong_t * len); /* TODO: do we use this? */

char * readEntireTextFile(const char * filename);

#endif /* __IO_H__ */