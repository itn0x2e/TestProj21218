#ifndef __IO_H__
#define __IO_H__

#include <stdio.h>
#include "types.h"


#define WRITE_BUF_SIZE (BUFSIZ)

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

uint_t countNewlines(const char * str); /* counts the occurrences of '\n' */ 


bool_t pfread(FILE * fd, int offset_from_begining, byte_t * buf, size_t buf_size);
bool_t pfwrite(FILE * fd, int offset_from_begining, byte_t * buf, size_t buf_size);
bool_t growFile(FILE * fd, size_t size);


#endif /* __IO_H__ */
