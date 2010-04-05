#ifndef __PASSWORD_GENERATOR_H__
#define __PASSWORD_GENERATOR_H__

#include "../../common/types.h"
#include "rule_segment.h"

typedef struct passwordGenerator_s {
	ruleSegment_t * ruleSegments;
	ulong_t * ruleSegmentAccumulativeSizes; // TODO: document that the offset is the first which is outside the range
	uint_t numRuleSegments;
	ulong_t size;
	ulong_t maxPasswordLength;
} passwordGenerator_t;

bool_t passwordGeneratorInitialize(passwordGenerator_t * self,
				   const char * rule,
				   const dictionary_t * dictionary);
void passwordGeneratorFinalize(passwordGenerator_t * self);
ulong_t passwordGeneratorGetSize(const passwordGenerator_t * self);
ulong_t passwordGeneratorGetMaxLength(const passwordGenerator_t * self);
const bool_t passwordGeneratorCalculatePassword(const passwordGenerator_t * self, ulong_t index, char * buf);

#endif /* __PASSWORD_GENERATOR_H__ */
