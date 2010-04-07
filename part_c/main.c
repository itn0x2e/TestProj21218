#include <stdio.h>
#include <stdlib.h>
#include "../common/misc.h"
#include "../password/dictionary.h"
#include "../password/password_generator.h"
#include "../password/random_password_enumerator.h"

bool_t createRainbowTable(
			passwordEnumerator_t * passEnumerator,
			char * password,

			BasicHashFunctionPtr hashFunc,

			ulong_t rainbowChainLen,

			char * hashTableFilePrefix,
			ulong_t nhashTableEntries,
			ulong_t nPairsPerBlock,
			bool_t enableFirstBlockCache,
			bool_t enableLastBlockCache);

int main(int agrc, char ** argv) {
	dictionary_t dictionary;
	passwordGenerator_t passwordGenerator;
	randomPasswordEnumerator_t randomPasswordEnumerator;
	char * buf = NULL;
	
	dictionaryInitialize(&dictionary, "hello\nworld");
	passwordGeneratorInitialize(&passwordGenerator, "||@&1-1|&1-1@", &dictionary);
	buf = (char *) malloc (passwordGeneratorGetMaxLength(&passwordGenerator) + 1);
	randomPasswordEnumeratorInitialize(&randomPasswordEnumerator,  &passwordGenerator, buf, 10 *passwordGeneratorGetSize(&passwordGenerator));
	
	/*while (passwordEnumeratorCalculateNextPassword((passwordEnumerator_t *) &randomPasswordEnumerator)) {
		printf("%s\n", buf);
	}*/

	return (!createRainbowTable((passwordEnumerator_t *) &randomPasswordEnumerator,
				    buf,
				    getHashFunFromName("MD5"),
				    5,
				    "rainbow_test",
				    100,
				    100,
				    TRUE,
				    TRUE));
}