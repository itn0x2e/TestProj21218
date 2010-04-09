#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "io.h"
#include "utils.h"
#include "../DEHT/DEHT.h"
#include "ui.h"

static bool_t isRuleValid(const char * rule);
static bool_t checkDEHTExistence(const char * prefix, bool_t shouldExist);
static bool_t checkFileExistence(const char * filename, bool_t shouldExist);
static const char * skipRangeRepresentation(const char * str);
static const char * skipNumRepresentation(const char * str);
static void setIniKey(const char ** keys, const char ** values, uint_t numKeys, const char * key, const char * value);
static bool_t verifyAllIniValuesSet(const char ** keys, const char ** values, uint_t numValues);

bool_t validateRule(const char * rule) {
	if (!isRuleValid(rule)) {
		fprintf(stderr, "Error: rule \"%s\" does not fit syntax.\n", rule);
		return FALSE;
	}
	
	return TRUE;
}

bool_t verifyDEHTExists(const char * prefix) {
	return checkDEHTExistence(prefix, TRUE);
}

bool_t verifyDEHTNotExist(const char * prefix) {
	return checkDEHTExistence(prefix, FALSE);
}

bool_t verifyFileExists(const char * filename) {
	return checkFileExistence(filename, TRUE);
}

bool_t verifyFileNotExist(const char * filename) {
	return checkFileExistence(filename, FALSE);
}

bool_t parseIni(char * content, const char ** keys, const char ** values, uint_t numKeys) {
	char * key = content;
	const char * delimeter = " = ";
	
	/* Initially set all pointers to NULL */
	memset(values, '\0', numKeys * sizeof(*values));
	
	while ('\0' != *key) {
		char * value;
		
		/* Find the end of the key representation */
		value = strchr(key, delimeter[0]);
		CHECK(NULL != value);
		CHECK(0 == strncmp(value, delimeter, strlen(delimeter)));
		
		/* Make the key representation null-terminated */
		*value = '\0';
		
		/* The value appears after the delimeter */
		value += strlen(delimeter);
		
		/* Set the value which matches the key */
		setIniKey(keys, values, numKeys, key, value);
		
		/* Move to end of line */
		key = strchr(value, '\n');
		if (NULL == key) {
			/* Value is already null-terminated */
			break;
		}
		
		/* Make the value null-terminated and advance to the next key */
		*key = '\0';
		++key;
	}
	
	if (!verifyAllIniValuesSet(keys, values, numKeys)) {
		return FALSE;
	}
	
	return TRUE;
	
LBL_ERROR:
	fprintf(stderr, "Error: configuration file corruption\n");
	return FALSE;
		
}

bool_t parseIniNum(const char * str, ulong_t * num) {
	if (1 != sscanf(str, "%lu", num)) {
		fprintf(stderr, "Error: configuration file corruption\n");
		return FALSE;
	}
	
	return TRUE;
}

void printIni(const char ** keys, const char ** values, uint_t numKeys) {
	uint_t i;
	
	for (i = 0; i < numKeys; ++i) {
		printf("%s = %s\n", keys[i], values[i]);
	}
}

bool_t parseHashFunName(BasicHashFunctionPtr * hashFunc, const char * name) {
	*hashFunc = getHashFunFromName(name);
	if (NULL == *hashFunc) {
		fprintf(stderr, "Error: Hash \"%s\" is not supported\n", name);
		return FALSE;
	}
	return TRUE;
}

bool_t readDictionaryFromFile(dictionary_t * dictionary, const char * filename) {
	bool_t ret = FALSE;
	char * rawDictionary = readEntireTextFile(filename);
	CHECK(NULL != rawDictionary);
	CHECK(dictionaryInitialize(dictionary, rawDictionary));
	ret = TRUE;

LBL_ERROR:
	FREE(rawDictionary);
	return ret;	
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

static bool_t checkDEHTExistence(const char * prefix, bool_t shouldExist)  {
	bool_t ret = FALSE;
	uint_t prefixLen = strlen(prefix);
	uint_t numFilenameBufChars = prefixLen + MAX(strlen(KEY_FILE_EXT), strlen(DATA_FILE_EXT)) + 1;
	char * filename = (char *) malloc (sizeof(char) * numFilenameBufChars);
	if (NULL == filename) {
		PERROR();
		return FALSE;
	}
	
	strcpy(filename, prefix);
	strcat(filename, KEY_FILE_EXT);
	CHECK(checkFileExistence(filename, shouldExist));
	
	strcpy(filename + prefixLen, DATA_FILE_EXT);
	CHECK(checkFileExistence(filename, shouldExist));
	
	ret = TRUE;
	
LBL_ERROR:
	FREE(filename);
	return ret;
}

static bool_t checkFileExistence(const char * filename, bool_t shouldExist)  {
	if (doesFileExist(filename)) {
		if (!shouldExist) {
			fprintf(stderr, "Error: File \"%s\" already exist\n", filename);
			return FALSE;
		}
	} else {
		if (shouldExist) {
			fprintf(stderr, "Error: File \"%s\" does not exist\n", filename);
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

static void setIniKey(const char ** keys, const char ** values, uint_t numKeys, const char * key, const char * value) {
	uint_t i;
	
	for(i = 0; i < numKeys; ++i) {
		if (0 == strcmp(key, keys[i])) {
			values[i] = value;
			return;
		}
	}
}

static bool_t verifyAllIniValuesSet(const char ** keys, const char ** values, uint_t numValues) {
	uint_t i;
	
	for(i = 0; i < numValues; ++i) {
		if (NULL == values[i]) {
			fprintf(stderr, "Error: key \"%s\" is missing from configuration file\n", keys[i]);
			return FALSE;
		}
	}
	
	return TRUE;
}
