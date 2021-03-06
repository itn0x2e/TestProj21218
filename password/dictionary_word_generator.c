#include "../common/utils.h"
#include "dictionary_word_generator.h"

void dictionaryWordGeneratorInitialize(dictionaryWordGenerator_t * self,
				       const dictionary_t * dictionary) {
	passwordPartGeneratorInitialize((passwordPartGenerator_t *) self);
	self->dictionary = dictionary;
}
