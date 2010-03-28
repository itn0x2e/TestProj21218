#ifndef EXHAUSTIVE_TABLE_GENERATOR_H_
#define EXHAUSTIVE_TABLE_GENERATOR_H_

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

#endif /*EXHAUSTIVE_TABLE_GENERATOR_H_*/
