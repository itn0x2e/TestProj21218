#ifndef __PASSWORD_GENERATOR_H__
#define __PASSWORD_GENERATOR_H__

#include "../common/types.h"
#include "rule_segment.h"

/**
 * A random-access generator of passwords which apply to a specified rule.
 *
 * This type is a composite of rule segments, separated in the rule by the
 * delimiter '|'. The range of passwords is the union of the ranges of all
 * those rule segments, and the size is equal to the summary.
 * Note that if there is a non-empty intersection between the segments,
 * passwords which apply for more than one rule segment are generated by
 * the generator for more than one index. Therefore, it is possible that
 * various indexes result in the same password.
 */
typedef struct passwordGenerator_s {
	/* The segments the union of which defines the rule */
	ruleSegment_t * ruleSegments;

	/* The i'th element in this array holds the accumulative size up to the
	 * i'th rule segment, inclusively.
	 * For each segment, this is equal to the first offset outside the range */
	ulong_t * ruleSegmentAccumulativeSizes;

	/* The number of rule segments */
	uint_t numRuleSegments;

	/* The total number of valid password indexes */
	ulong_t size;

	/* The length of the longest password, not including the terminating null */
	ulong_t maxPasswordLength;
} passwordGenerator_t;

/**
 * Initialization function.
 *
 * @param	self	The password generator to initialize
 * @param	rule	A string representation of the rule
 * @param	dictionary	The dictionary to use whenever specified by the rule
 *
 * @pre		self != NULL
 * @pre		rule has a valid syntax
 * @pre		dictionary != NULL
 * @pre		dictionary is initialized
 *
 * @return 	TRUE upon success, otherwise FALSE.
 *
 * @note	Avoid invoking the finalization function if the return value is FALSE.
 */
bool_t passwordGeneratorInitialize(passwordGenerator_t * self,
				   const char * rule,
				   const dictionary_t * dictionary);

/**
 * Finalization function.
 *
 * @param	self	The password generator to finalize
 * @pre		self != NULL
 * @pre		self is initialized.
 */
void passwordGeneratorFinalize(passwordGenerator_t * self);

/**
 * @param	self	A password generator
 * @pre		self != NULL
 * @pre		self is initialized.
 *
 * @return	The total number of valid password indexes
 */
ulong_t passwordGeneratorGetSize(const passwordGenerator_t * self);

/**
 * @param	self	A password generator
 * @pre		self != NULL
 * @pre		self is initialized.
 *
 * @return	The total number of valid password indexes
 */
ulong_t passwordGeneratorGetMaxLength(const passwordGenerator_t * self);

/**
 * Calculates the password in a specified index and stores it into a specified
 * buffer.
 *
 * @param	self	A password generator
 * @param	index	The index
 * @param	buf		A buffer into which to write the corresponding password
 *
 * @pre		self != NULL
 * @pre		self is initialized.
 * @pre		index < passwordGeneratorGetSize(self)
 * @pre		buf != NULL
 * @pre		buf is large enough to contain, including a terminating null, any
 * 			password generated by the generator.
 */
void passwordGeneratorCalculatePassword(const passwordGenerator_t * self, ulong_t index, char * buf);

#endif /* __PASSWORD_GENERATOR_H__ */
