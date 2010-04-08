#include <stdio.h>
#include <stdlib.h>
#include "../../common/misc.h"
#include "../../password/dictionary.h"
#include "../../password/password_generator.h"
#include "../../password/random_password_enumerator.h"
#include "../rainbow_table.h"


bool_t removeKeyFile(void)
{
	return (0 == remove("rainbow_test.key"));
}

bool_t removeDataFile(void)
{
	return (0 == remove("rainbow_test.data"));
}


bool_t removeFiles(void)
{
	bool_t ret = TRUE;

	/* try doing both anyway */	
	ret &= removeKeyFile();
	ret &= removeDataFile();

	return ret;
}


int main(int agrc, char ** argv) {
	dictionary_t dictionary;
	passwordGenerator_t passwordGenerator;
	randomPasswordEnumerator_t randomPasswordEnumerator;
	char * generatorBuf = NULL;
	char * enumeratorBuf = NULL;
	

	(void) removeFiles();

	dictionaryInitialize(&dictionary, "hello\nworld");
	passwordGeneratorInitialize(&passwordGenerator, "||@&1-1|&1-1@", &dictionary);
	generatorBuf = (char *) malloc (passwordGeneratorGetMaxLength(&passwordGenerator) + 1);
	enumeratorBuf = (char *) malloc (passwordGeneratorGetMaxLength(&passwordGenerator) + 1);
	randomPasswordEnumeratorInitialize(&randomPasswordEnumerator,  &passwordGenerator, enumeratorBuf, 10 *passwordGeneratorGetSize(&passwordGenerator));
	
	/*while (passwordEnumeratorCalculateNextPassword((passwordEnumerator_t *) &randomPasswordEnumerator)) {
		printf("%s\n", buf);
	}*/

	return (!RT_generate((passwordEnumerator_t *) &randomPasswordEnumerator, &passwordGenerator,
				    enumeratorBuf, generatorBuf,
				    getHashFunFromName("MD5"),
				    5,
				    "rainbow_test",
				    100,
				    100,
				    TRUE,
				    TRUE));
}
