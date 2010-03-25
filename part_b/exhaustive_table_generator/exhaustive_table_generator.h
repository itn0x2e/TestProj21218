#ifndef EXHAUSTIVE_TABLE_GENERATOR_H_
#define EXHAUSTIVE_TABLE_GENERATOR_H_

/* Used only on non-predicate functions */
typedef enum ret_e {
	ERR_FAIL,
	ERR_OK,
	ERR_EOF
} ret_t;


int main(int argc, char** argv);

/**
 * Calculates a password corresponding to the index, according to the rule.
 * This function allocates memory for the null-terminated string that it returns.
 */
char* calculatePassword(unsigned long index);

/**
 * findNextWordInDict function.
 * Finds the next word in the supplied raw dict. Used for the dict vectorification process
 * Built to support iterating over both the original (one big string) dict format, 
 * and the new (split-strings) dictionary formats.
 *
 * @param 	rawDict	pointer to the current location in the dictionary, 
 *		from which to seek forward
 * @param 	next	pointer that will be set the next word in the dictionary,
 *			or NULL if no such word could be found
 *		nextLen pointer that will receive the length of the new word
 *
 * @return	ERR_OK upon success, ERR_FAIL otherwise
 */
ret_t findNextWordInDict(char * rawDict, char ** next, uint_t * nextLen);

/**
 * createDictStringVector function.
 * Scans a positive integer from the user and outputs its factorization.
 *
 * @param 	rawDict		pointer to raw dictionary (as read by load_file - one big string)
 * 				This buffer will undergo modification, so be warned.
 * @param 	parsedDict	will receive a pointer to an argv-style array pointing to
 *				each word. Words have been null-terminated.
 *				When you are done with this vector, you must call free() to
 *				release it. Do not call free for rawDict until you are done,
 *				as the pointers in *parsedDict point to strings inside rawDict!
 *
 * @return	ERR_OK upon success, ERR_FAIL otherwise
 *
 * @note 	Read rawDict's & parsedDict's description once more. Make sure you understand them
 */
ret_t createDictStringVector(char * rawDict, char *** parsedDict);


#endif /*EXHAUSTIVE_TABLE_GENERATOR_H_*/
