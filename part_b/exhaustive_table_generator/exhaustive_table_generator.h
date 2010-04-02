#ifndef __EXHAUSTIVE_TABLE_GENERATOR_H__
#define __EXHAUSTIVE_TABLE_GENERATOR_H__

#define PAIRS_PER_BLOCK (7)
#define BYTES_PER_SIGNATURE (8)
#define NUM_TABLE_ENTRIES (65536) /* 2^16 */

int main(int argc, char** argv);

int hashKeyIntoTable(const unsigned char * keyBuf,
		     int keySizeof,
		     int nTableSize);

int hashKeyforEfficientComparison(const unsigned char * keyBuf,
				  int keySizeof,
				  unsigned char *validationKeyBuf); 

#endif /* __EXHAUSTIVE_TABLE_GENERATOR_H__ */
