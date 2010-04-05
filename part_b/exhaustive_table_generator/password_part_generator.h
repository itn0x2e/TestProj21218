#ifndef __PASSWORD_PART_GENERATOR_H_
#define __PASSWORD_PART_GENERATOR_H_

#include "../../common/types.h"
#include "dictionary.h"

struct passwordPartGenerator_s;

typedef char * (* passwordPartGeneratorCalcPassFuncPtr_t) (const struct passwordPartGenerator_s *, ulong_t, char *);
typedef void (* passwordPartGeneratorFinalizeFuncPtr_t) (struct passwordPartGenerator_s *);

typedef struct passwordPartGenerator_s {
	ulong_t size;
	ulong_t maxLength;
	passwordPartGeneratorCalcPassFuncPtr_t calcPassFunc;
	passwordPartGeneratorFinalizeFuncPtr_t finalizeFunc;
} passwordPartGenerator_t;

passwordPartGenerator_t * passwordPartGeneratorCreate(const char * rulePart, const dictionary_t * dictionary);
void passwordPartGeneratorInitialize(passwordPartGenerator_t * self);
void passwordPartGeneratorFinalize(passwordPartGenerator_t * self);
ulong_t passwordPartGeneratorGetSize(const passwordPartGenerator_t * self);
ulong_t passwordPartGeneratorGetMaxLength(const passwordPartGenerator_t * self);
char * passwordPartGeneratorCalculatePassword(const passwordPartGenerator_t * self, ulong_t index, char * buf);

#endif /*__PASSWORD_PART_GENERATOR_H_*/
