#include <stdio.h>
#include "constants.h"
#include "types.h"

/**
 * @param line
 *
 * @pre line != NULL
 * @pre line is allocated for at least MAX_LINE_LEN chars
 */
bool_t readPrompt(char * line) {
	do {
		printf(">>");
		if (NULL == fgets(line, sizeof(line), stdin)) {
			return FALSE;
		}
	} while ('\n' == line[0]);
	
	return TRUE;
}
