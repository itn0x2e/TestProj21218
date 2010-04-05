#include <string.h>
#include <stdlib.h>
#include "../../common/util.h"
#include "password_generator.h"

static bool_t parseRule(passwordGenerator_t * self, const char * rule, const dictionary_t * dictionary);
				  
bool_t passwordGeneratorInitialize(passwordGenerator_t * self,
				   const char * rule,
				   const dictionary_t * dictionary) {
	uint_t i;
	self->ruleSegmentAccumulativeSizes = NULL; // TODO: this is important for d-tor
	CHECK(parseRule(self, rule, dictionary));

	self->maxPasswordLength = 0;
	self->size = 0;

	for (i = 0; i < self->numRuleSegments; ++i) {
		const ruleSegment_t * ruleSegment = self->ruleSegments + i;
		ulong_t currMaxLength = ruleSegmentGetMaxLength(ruleSegment);
		if (self->maxPasswordLength < currMaxLength) {
			self->maxPasswordLength = currMaxLength;
		}
		self->size += ruleSegmentGetSize(ruleSegment);
	}

	if (self->numRuleSegments > 0) {
		self->ruleSegmentAccumulativeSizes = (ulong_t *) malloc(self->numRuleSegments * sizeof(ulong_t));
		CHECK(NULL != self->ruleSegmentAccumulativeSizes);

		self->ruleSegmentAccumulativeSizes[0] = ruleSegmentGetSize(self->ruleSegments + 0);

		for (i = 1; i < self->numRuleSegments; ++i) {
			self->ruleSegmentAccumulativeSizes[i] =
					self->ruleSegmentAccumulativeSizes[i - 1] +
					ruleSegmentGetSize(self->ruleSegments + i);
		}
	}

	return TRUE;

LBL_ERROR:
	passwordGeneratorFinalize(self);
	return FALSE;
}

void passwordGeneratorFinalize(passwordGenerator_t * self) {
	uint_t i;
	// TODO: make sure that parseRule sets numRuleSegments to the actual number already initialized if it fails.
	for (i = 0; i < self->numRuleSegments; ++i) {
		ruleSegmentFinalize(self->ruleSegments + i);
	}

	FREE(self->ruleSegments);
	FREE(self->ruleSegmentAccumulativeSizes);
}

inline ulong_t passwordGeneratorGetSize(const passwordGenerator_t * self) {
	return self->size;
}

inline ulong_t passwordGeneratorGetMaxLength(const passwordGenerator_t * self) {
	return self->maxPasswordLength;
}

const bool_t passwordGeneratorCalculatePassword(const passwordGenerator_t * self, ulong_t index, char * buf) {
	uint_t i;

	ASSERT(index < self->size);

	// TODO: consider binary search
	for (i = 0; i < self->numRuleSegments; ++i) {
		if (index < self->ruleSegmentAccumulativeSizes[i]) {
			ulong_t relativeIndex = index;
			if (i > 0) {
				relativeIndex -= self->ruleSegmentAccumulativeSizes[i - 1];
			}

			return ruleSegmentCalculatePassword(self->ruleSegments + i, relativeIndex, buf);
		}
	}

	// This should never be reached;
	return FALSE;
}

static bool_t parseRule(passwordGenerator_t * self, const char * rule, const dictionary_t * dictionary) {
	const char * ruleSegmentStr = rule;
	uint_t i;
	
	self->numRuleSegments = 0;
	while (ruleSegmentStr != NULL) {
		self->numRuleSegments++;
		ruleSegmentStr = strchr(ruleSegmentStr, '|');
		if (NULL != ruleSegmentStr) {
			ruleSegmentStr += 1;
		}
	}
	
	self->ruleSegments = (ruleSegment_t *) malloc(self->numRuleSegments * sizeof(ruleSegment_t));
			
	if (NULL == self->ruleSegments) {
		self->numRuleSegments = 0;
		return FALSE;
	}
	
	ruleSegmentStr = rule;
	
	for (i = 0; i < self->numRuleSegments; ++i) {
		CHECK(ruleSegmentInitialize(self->ruleSegments + i, ruleSegmentStr, dictionary));
		ruleSegmentStr = strchr(ruleSegmentStr, '|') + 1;
	}
	
	return TRUE;

LBL_ERROR:
	/* Set numRuleSegments to the number already initialized, for the finalization function to know which should be finalized. */
	self->numRuleSegments = i;

	return FALSE;
}
