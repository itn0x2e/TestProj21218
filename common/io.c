#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "utils.h"
#include "io.h"

bool_t readLine(FILE * stream, char * line) {
	/* We assume that the line isn't longer than MAX_LINE_LEN */
	if (NULL == fgets(line, MAX_LINE_LEN, stream)) {
		return FALSE;
	}
	
	/* Remove the trailing '\n' from the end of the line */
	if ('\n' == line[strlen(line) - 1]) { 
		line[strlen(line) - 1] = '\0';
	}
	
	return TRUE;
}

bool_t doesFileExist(const char * filename) {
	FILE * file = fopen(filename, "r");
	if (NULL != file) {
		fclose(file);
		return TRUE;
	}
	return FALSE;
}

byte_t * readEntireBinaryFile(const char * filename, ulong_t * len) {
	byte_t * res = NULL;
	FILE * fp = NULL;

	fp = fopen(filename,"rb");
	CHECK(NULL != fp);

	fseek(fp,0,SEEK_END);
	*len = ftell(fp);

	res = (byte_t *) malloc(*len);
	CHECK(res != NULL);

	fseek(fp,0,SEEK_SET);
  
	CHECK(1 == fread(res, *len, 1, fp));
  
	FCLOSE(fp);
	return res;
	
LBL_ERROR:
	perror(filename);
	FREE(res);
	FCLOSE(fp);
	return NULL;
}

char * readEntireTextFile(const char * filename) {
	size_t len;
	char * res = NULL;
	FILE * fp = NULL;

	fp = fopen(filename,"r");
	CHECK(NULL != fp);

	fseek(fp,0,SEEK_END);
	len = ftell(fp);

	res = (char *) malloc(len + 1);
	CHECK(res != NULL);

	fseek(fp,0,SEEK_SET);
  
	CHECK(1 == fread(res, len, 1, fp));
	
	FCLOSE(fp);
	
	res[len] = '\0';
	
	return res;
	
LBL_ERROR:
	perror(filename);
	FREE(res);
	FCLOSE(fp);
	return NULL;
}

uint_t countNewlines(const char * str) {
	uint_t count = 0;
	
	str = strchr(str, '\n');
	
	while (NULL != str) {
		++count;
		str = strchr(str + 1, '\n');
	}
	
	return count;
}


bool_t pfread(FILE * fd, int offset_from_begining, byte_t * buf, size_t buf_size)
{
	bool_t ret = FALSE;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != fd);
	CHECK(NULL != buf);

	/* We attempt a flush before moving the file pointer, since there may be data pending the buffer */
	/* We ignore the return value, since we should try it anyway - it may work anyway */
	(void) fflush(fd);

	CHECK(0 == fseek(fd, offset_from_begining, SEEK_SET));

	/* read */
	CHECK(buf_size == fread(buf, 1, buf_size, fd));

	ret = TRUE;

	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}

bool_t pfwrite(FILE * fd, int offset_from_begining, byte_t * buf, size_t buf_size)
{
	bool_t ret = FALSE;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != fd);
	CHECK(NULL != buf);

	/* We attempt a flush before moving the file pointer, since there may be data pending the buffer */
	/* We ignore the return value, since we should try it anyway - it may work anyway */
	(void) fflush(fd);

	CHECK(0 == fseek(fd, offset_from_begining, SEEK_SET));

	/* write */
	CHECK(buf_size == fwrite(buf, 1, buf_size, fd));

	ret = TRUE;

	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;
}

bool_t growFile(FILE * fd, size_t size)
{
	bool_t ret = FALSE;

	byte_t zeroBuf[WRITE_BUF_SIZE];
	ulong_t writeSize = WRITE_BUF_SIZE;

	ulong_t bytesWritten = 0;

	TRACE_FUNC_ENTRY();
	CHECK(NULL != fd);

	/* init buf */
	memset(zeroBuf, 0, sizeof(zeroBuf));

	/* move to file end (ignore flush errors) */
	(void) fflush(fd);
	CHECK(0 == fseek(fd, 0, SEEK_END));

	/* write in chunks */
	if (writeSize > size) {
		writeSize = size;
	}
	for (bytesWritten = 0;  bytesWritten < size; bytesWritten += writeSize) {

		CHECK(writeSize == fwrite(zeroBuf, 1, writeSize, fd));
		
		if (bytesWritten + writeSize > size) {
			writeSize = size - bytesWritten;
		}
	}

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;

}

bool_t removeFile(char * fileName)
{
	bool_t ret = FALSE;
	FILE * fd = NULL;

	TRACE_FUNC_ENTRY();

	CHECK(NULL != fileName);

	/* First, check if the file is present */
	/*! Due to the fact we were limited to using the ANSI-C std-lib, this is the best way we could
	   check a file's existance.  Using the unix API would be much better (access / stat) !*/
	fd = fopen(fileName, "rb");
	if (NULL == fd) {
		return TRUE;
	}

	FCLOSE(fd);

	/* remove the file */
	CHECK_MSG(fileName, (0 == remove(fileName)));

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	ret = FALSE;
	TRACE_FUNC_ERROR();

LBL_CLEANUP:
	TRACE_FUNC_EXIT();
	return ret;

}
