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

/**
 * @param filename - a name of a file
 * @return	whether a file with the specified name exists.
 */
bool_t doesFileExist(const char * filename);

/**
 * @param filename - a name of a text file
 * @return	a newly allocated null-terminated string conataining its content,
 *		or FALSE upon failure
 */
char * readEntireTextFile(const char * filename);

/**
 * @param str - A null-terminated string
 *
 * @return The number of '\n' occurrences
 */
uint_t countNewlines(const char * str);

/**
 * Read data from a file at the wanted offset
 * A wrapper for fread. This function first positions the file pointer at the wanted
 * position, making "random access read" easier.
 *
 * @param fd - file descriptor to read from
 * @param offset_from_begining - offset from file begining (result of ftell())
 * @param buf - buffer to read into
 * @param buf_size - number of bytes to read. The file must contain enought bytes
 *		    for this operation to succeed.
 *
 * @return TRUE on success, FALSE otherwise.
 *
 */
bool_t pfread(FILE * fd, int offset_from_begining, byte_t * buf, size_t buf_size);

/**
 * Write data to a file at the wanted offset
 * A wraper for fwrite. This function first positions the file pointer at the wanted
 * position, making "random access write" easier.
 *
 * @param fd - file descriptor to write to
 * @param offset_from_begining - offset from file begining (result of ftell())
 * @param buf - buffer to write to file
 * @param buf_size - number of bytes to write.
 *
 * @return TRUE on success, FALSE otherwise.
 *
 */
bool_t pfwrite(FILE * fd, int offset_from_begining, byte_t * buf, size_t buf_size);

/**
 * Append 'size' bytes to the end of the file.
 * This function will append 'size' bytes to the end of the files. The
 * added bytes are initialized to zero.
 *
 * @param fd - file descriptor to write to
 * @param size - number of bytes to add to the file
 *
 * @return TRUE on success, FALSE otherwise.
 *
 */
bool_t growFile(FILE * fd, size_t size);

/**
 * Remove (delete) a file 
 * This function will tries to guarantee that a file will not exist.
 * it does so by first checking if the file is present, and if it is,
 * calling libstdc's remove().
 *
 * @param fileName - path to file to remove
 *
 * @return If the return value of the function is TRUE, the file should not be present.
 *      If the operation failed due to some reason, the result will be FALSE.
 *
 */
bool_t removeFile(char * fileName);

#endif /* __IO_H__ */
