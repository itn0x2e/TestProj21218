

static bool_t printSeeds(RainbowTable_t * rt, FILE * fd)
{
	/* print seeds used by rt */

	return TRUE;
}


static bool_t printPasswords(RainbowTable_t * rt, FILE * fd)
{
	/* print pws used by rt */

	return TRUE;
}


bool_t RT_print(FILE * file1, FILE * file2,
		const passwordGenerator_t * passGenerator,
		char * generatorPassword,

		const char * hashTableFilePrefix,
		bool_t enableFirstBlockCache,
		bool_t enableLastBlockCache)
{
	RainbowTable_t * rt = NULL;
	ulong_t bucketId = 0;

	/* Open rainbow table */
	rt = RT_open(passGenerator, generatorPassword, hashTableFilePrefix, enableFirstBlockCache, enableLastBlockCache);
	CHECK(NULL != rt);

	CHECK(printSeeds(rt, file1));

	for (bucketId = 0;  bucketId <  rt->hashTable->header.numEntriesInHashTable; ++bucketId) {
		/* scan bucket */
	/*	CHECK(printSeeds(rt, file1));*/

		
	}


}
