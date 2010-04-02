#ifndef __RULE_SEGMENT_H__
#define __RULE_SEGMENT_H__

#include "../../common/types.h"
#include "dictionary.h"
#include "password_part_generator.h"

const char * RULE_SEGMENT_RULE_TOKENS = "?&!%@#$";

typedef struct ruleSegment_s {
	passwordPartGenerator_t ** passwordPartGenerators;
	uint_t numPasswordPartGenerators;
	ulong_t size;
	ulong_t maxPasswordLength;
} ruleSegment_t;

bool_t ruleSegmentInitialize(ruleSegment_t * self, const char * ruleSegmentStr, const dictionary_t * dictionary);
void ruleSegmentFinalize(ruleSegment_t * self);
inline ulong_t ruleSegmentGetSize(const ruleSegment_t * self);
inline ulong_t ruleSegmentGetMaxLength(const ruleSegment_t * self);
bool_t ruleSegmentCalculatePassword(const ruleSegment_t * self, ulong_t index, char * buf);

#endif /*__RULE_SEGMENT_H__*/
