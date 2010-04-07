#include <string.h>
#include "constants.h"
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
