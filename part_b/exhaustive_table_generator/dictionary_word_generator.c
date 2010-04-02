#include "../../common/util.h"
#include "dictionary_word_generator.h"

inline void dictionaryWordGeneratorInitialize(dictionaryWordGenerator_t * self, dictionary_t * dictionary) {
	passwordPartGeneratorInitialize((passwordPartGenerator_t *) self);
	self->dictionary = dictionary;
}
