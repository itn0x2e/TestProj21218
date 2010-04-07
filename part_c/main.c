#include <stdio.h>
#include <stdlib.h>
#include "../common/misc.h"
#include "../password/all_password_enumerator.h"
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
	allPasswordEnumerator_t allPasswordEnumerator;
	randomPasswordEnumerator_t randomPasswordEnumerator;
	char * buf = NULL;
	
	dictionaryInitialize(&dictionary, "");
	passwordGeneratorInitialize(&passwordGenerator, "&0-10", &dictionary);
	buf = (char *) malloc (passwordGeneratorGetMaxLength(&passwordGenerator) + 1);
	allPasswordEnumeratorInitialize(&allPasswordEnumerator, &passwordGenerator, buf);
	randomPasswordEnumeratorInitialize(&randomPasswordEnumerator,  &passwordGenerator, buf, 100);
	
	while (passwordEnumeratorCalculateNextPassword((passwordEnumerator_t *) &randomPasswordEnumerator)) {
		printf("%s\n", buf);
	}
	
	return 0;
	/*createRainbowTable((passwordEnumerator_t *) &allPasswordEnumerator,
			   buf,
			   getHashFunFromName("MD5"),*/
			   
}