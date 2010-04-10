#include <string.h>
#include "../common/utils.h"
#include "rule_segment.h"

static bool_t parseRuleSegment(ruleSegment_t * self, const char * ruleSegmentStr, const dictionary_t * dictionary);

bool_t ruleSegmentInitialize(ruleSegment_t * self,
							const char * ruleSegmentStr,
							const dictionary_t * dictionary) {
	uint_t i;

	CHECK(parseRuleSegment(self, ruleSegmentStr, dictionary));
	self->maxPasswordLength = 0;

	if (0 == self->numPasswordPartGenerators) {
		self->size = 0;
		return TRUE;
	}

	self->size = 1;

	for (i = 0; i < self->numPasswordPartGenerators; ++i) {
		const passwordPartGenerator_t * partGenerator = self->passwordPartGenerators[i];
		self->maxPasswordLength += passwordPartGeneratorGetMaxLength(partGenerator);
		self->size *= passwordPartGeneratorGetSize(partGenerator);
	}

	return TRUE;

LBL_ERROR:
	ruleSegmentFinalize(self);
	return FALSE;
}

void ruleSegmentFinalize(ruleSegment_t * self)  {
	uint_t i;

	for (i = 0; i < self->numPasswordPartGenerators; ++i) {
		passwordPartGenerator_t * generator = self->passwordPartGenerators[i];
		passwordPartGeneratorFinalize(generator);
		FREE(generator);
	}

	FREE(self->passwordPartGenerators);
}

ulong_t ruleSegmentGetSize(const ruleSegment_t * self) {
	return self->size;
}

ulong_t ruleSegmentGetMaxLength(const ruleSegment_t * self) {
	return self->maxPasswordLength;
}

void ruleSegmentCalculatePassword(const ruleSegment_t * self, ulong_t index, char * buf) {
	uint_t i;
	ASSERT(index < self->size);

	for (i = 0; i < self->numPasswordPartGenerators; ++i) {
		const passwordPartGenerator_t * partGenerator = self->passwordPartGenerators[i];
		ulong_t size = passwordPartGeneratorGetSize(partGenerator);

		buf = passwordPartGeneratorCalculatePassword(partGenerator, index % size, buf);

		index /= size;
	}

	*buf = '\0';
}

static bool_t parseRuleSegment(
		ruleSegment_t * self,
		const char * ruleSegmentStr,
		const dictionary_t * dictionary) {

	const char * ruleSegmentEnd = strchr(ruleSegmentStr, '|');
	const char * currRulePart = strpbrk(ruleSegmentStr, RULE_SEGMENT_RULE_TOKENS);
	uint_t numRuleParts = 0;
	uint_t i;
	
	self->numPasswordPartGenerators = 0;

	while (currRulePart != NULL) {
		/* if we exeeded the current rule segment, stop */
		if ((NULL != ruleSegmentEnd) && (currRulePart > ruleSegmentEnd)) {
			break;
		}
		numRuleParts++;
		currRulePart = strpbrk(currRulePart + 1, RULE_SEGMENT_RULE_TOKENS);
	}
	
	/* Note: calloc is used s.t. all pointers are initially NULL. */
	self->passwordPartGenerators = (passwordPartGenerator_t **)
		calloc(numRuleParts, sizeof(passwordPartGenerator_t *));
			
	if (NULL == self->passwordPartGenerators) {
		/* Set numPasswordPartGenerators to 0 in order to maintain the
		 * invariant that numPasswordPartGenerators equals the number of
		 * allocated generators. */
		return FALSE;
	}
	
	/* Go back to the first rule part. */
	currRulePart = strpbrk(ruleSegmentStr, RULE_SEGMENT_RULE_TOKENS);
	
	/* For each rule part, create a password part generator. */
	for (i = 0; i < numRuleParts; ++i) {
		self->passwordPartGenerators[i] =
			passwordPartGeneratorCreate(currRulePart, dictionary);
		
		if (NULL == self->passwordPartGenerators[i]) {
			/* Set numPasswordPartGenerators to the number of generators actually
			 * created, in order to maintain the invariant that numPasswordPartGenerators
			 * equals the number of allocated generators. */
			self->numPasswordPartGenerators = i;

			return FALSE;
		}

		currRulePart = strpbrk(currRulePart + 1, RULE_SEGMENT_RULE_TOKENS);
	}
	
	self->numPasswordPartGenerators = numRuleParts;

	return TRUE;
}
