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