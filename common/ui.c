#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "io.h"
#include "ui.h"

static bool_t isRuleValid(const char * rule);
static const char * skipRangeRepresentation(const char * str);
static const char * skipNumRepresentation(const char * str);

bool_t validateRule(const char * rule) {
	if (!isRuleValid(rule)) {
		fprintf(stderr, "Error: rule \"%s\" does not fit syntax.\n", rule);
		return FALSE;
	}
	
	return TRUE;
}

bool_t validateFileNotExist(const char * filename) {
	if (doesFileExist(filename)) {
		fprintf(stderr, "Error: File \"%s\" already exist\n", filename);
		return FALSE;
	}
	
	return TRUE;
}

bool_t parseHashFunName(BasicHashFunctionPtr * hashFunc, const char * name) {
	*hashFunc = getHashFunFromName(name);
	if (NULL == *hashFunc) {
		fprintf(stderr, "Error: Hash \"%s\" is not supported\n", name);
		return FALSE;
	}
	return TRUE;
}

bool_t readPrompt(char * line) {
	/* Repeat as long as empty lines are read */
	do {
		printf(">>");
		if (!readLine(stdin, line)) {
			return FALSE;
		}
	} while ('\0' == *line);
	
	return TRUE;
}
/* TODO
bool_t parseIni(char * content, const char ** keys, const char ** values, int_t numKeys) {
	char * key = content;
	
	while (('\n' != *key) && ('\0' != *key)) {
		
}
 */
static bool_t isRuleValid(const char * rule) {
	uint_t rangeBegin, rangeEnd;
	
	while ('\0' != *rule) {
		switch (*rule) {
		case '?':
		case '&':
		case '!':
		case '%':
			/* Validate the range, which must follow these symbols */
			if((2 != sscanf(rule + 1, "%u-%u", &rangeBegin, &rangeEnd)) || (rangeBegin > rangeEnd)) {
				return FALSE;
			}
			
			/* Advance to the next character after the range */
			rule = skipRangeRepresentation(rule + 1);
			break;
		case '@':
		case '#':
		case '$':
		case '|':
			rule += 1;
			break;
		default:
			/* An invalid symbol has been encounterd */
			return FALSE;
		}
	}
	
	return TRUE;
}

static const char * skipRangeRepresentation(const char * str) {
	/* Skip the first number */
	str = skipNumRepresentation(str);
	
	/* Skip the '-' delimeter */
	str += 1;
	
	/* Skip the second number */
	return skipNumRepresentation(str);
}

static const char * skipNumRepresentation(const char * str) {
	while(isdigit(*str)) {
		++str;
	}
	
	return str;
}
