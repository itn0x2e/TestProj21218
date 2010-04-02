#include "exhaustive_table_generator.h"

ret_t findNextWordInRawDict(char * rawDict, char ** next, uint_t * nextLen)
{
	uint_t i = 0;
	char * rawDict_end = rawDict + strlen(rawDict) + 1;

	/* skip any loose terminators at the string's begining */
	while ((rawDict_end > rawDict) && (0x30 > *rawDict)) {
		++rawDict;
	}

	*next = NULL;
	*nextLen = 0;

	/* Scan the string, looking for the word's ending */
	for (i = 0; ((i < strlen(rawDict)) && (0x30 <= rawDict[i])); ++i) {
		/* Do nothing here */
	}

	if (strlen(rawDict) <= i) {
		return ERR_FAIL;
	}

	*next = rawDict;
	*nextLen = i;

	return ERR_OK;
}

ret_t createDictStringVector(char * rawDict, char *** parsedDict)
{
	uint_t dictLen = 0;
	char * currWord = rawDict;
	uint_t currWordLen = 0;

	*parsedDict = NULL;

	dictLen = 0;
	do {
		++dictLen;
	} while (findNextWordInRawDict(currWord + currWordLen + 1, &currWord, &currWordLen));


	/* Alloc the string pointer vector. We alloc one extra member for a NULL terminator */
	*parsedDict = (char **) malloc((dictLen+1) * sizeof(**parsedDict));
	if (NULL == *parsedDict) {
		return ERR_FAIL;
	}
	memset(*parsedDict, (dictLen+1) * sizeof(**parsedDict), 0);

	/* Now scan the raw dict again, this time updating the resulting array and splitting the strings */
	currWord = rawDict;
	dictLen = 0;
	findNextWordInRawDict(currWord, &currWord, &currWordLen);
	do {
		(*parsedDict)[dictLen] = currWord;
		(*parsedDict)[dictLen][currWordLen] = '\0';

		/* we want our dict to be in lower-case */
		(void) strToLower((*parsedDict)[dictLen]);

		++dictLen;

	} while (findNextWordInRawDict(currWord + currWordLen + 1, &currWord, &currWordLen));
	
	return ERR_OK;	
}

