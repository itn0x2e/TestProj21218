#include <ctype.h>
#include <stdio.h>
#include "constants.h"
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
/*
hashKeyIntoTableFunctionPtr getHashFunFromName(const char * name) {
	if (strcmp(name, "MD5")) {
		return 
}
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
