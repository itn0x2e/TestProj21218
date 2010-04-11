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

bool_t doesFileExist(const char * filename);

byte_t * readEntireBinaryFile(const char * filename, ulong_t * len); /* TODO: do we use this? */

char * readEntireTextFile(const char * filename);

uint_t countNewlines(const char * str); /* counts the occurrences of '\n' */ 


/**
* Read data from a file at the wanted offset
* Function desc: A wraper for fread. This function first positions the file pointer at the wanted
*		 position, making "random access read" easier.
*
* @param fd - file descriptor to read from
* @param offset_from_begining - offset from file begining (result of ftell())
* @param buf - buffer to read into
* @param buf_size - number of bytes to read. The file must contain enought bytes
*		    for this operation to succeed.
*
* @ret TRUE on success, FALSE otherwise.
*
*/
bool_t pfread(FILE * fd, int offset_from_begining, byte_t * buf, size_t buf_size);

/**
* Write data to a file at the wanted offset
* Function desc: A wraper for fwrite. This function first positions the file pointer at the wanted
*		 position, making "random access write" easier.
*
* @param fd - file descriptor to write to
* @param offset_from_begining - offset from file begining (result of ftell())
* @param buf - buffer to write to file
* @param buf_size - number of bytes to write.
*
* @ret TRUE on success, FALSE otherwise.
*
*/
bool_t pfwrite(FILE * fd, int offset_from_begining, byte_t * buf, size_t buf_size);

/**
* Append 'size' bytes to the end of the file.
* Function desc: This function will append 'size' bytes to the end of the files. The
*		 added bytes are initialized to zero.
*
* @param fd - file descriptor to write to
* @param size - number of bytes to add to the file
*
* @ret TRUE on success, FALSE otherwise.
*
*/
bool_t growFile(FILE * fd, size_t size);


/**
* Remove (delete) a file 
* Function desc: This function will tries to guarantee that a file will not exist.
*		 it does so by first checking if the file is present, and if it is,
*		 calling libstdc's remove().
*
* @param fileName - path to file to remove
*
* @ret If the return value of the function is TRUE, the file should not be present.
*      If the operation failed due to some reason, the result will be FALSE.
*
*/
bool_t removeFile(char * fileName);

#endif /* __IO_H__ */
