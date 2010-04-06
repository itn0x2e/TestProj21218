#include "ui.h"

bool_t readPrompt(char * line) {
	do {
		printf(">>");
		if (NULL == fgets(line, sizeof(line), stdin)) {
			return FALSE;
		}
	} while ('\n' == line[0]);
	
	return TRUE;
}
 
