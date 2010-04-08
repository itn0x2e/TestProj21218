#ifndef __DEHT_HASH_FUNCS_H__
#define __DEHT_HASH_FUNCS_H__


#define TABLE_INDEX_SEED ("\x6d\xae\x89\x87\xde\x43\x46\x61\x22\xcb\x65\xdd\x3f\xeb\x42\x35\x8f\x82\x01\x0b\xc4\xef\x63\x29\x48\x7c\xc1\x57\x61\xa3\x67\x22")
/*#define VALIDATION_SEED ("\xcb\xd8\xa2\xaf\xf6\x44\xb7\x83\xa7\x0a\x33\x10\xfc\x8c\x4c\xa4\xd5\x14\x5f\x5d\x3b\x2a\xe3\x07\xd5\x86\xbc\xef\x50\x09\xb3\xab")*/
#define VALIDATION_SEED ("mitsi")


int DEHT_keyToTableIndexHasher(const unsigned char * key, int keySize, int tableSize);
int DEHT_keyToValidationKeyHasher64(const unsigned char * key,int keySize, unsigned char * resBuf);

int DEHT_keyToValidationKeyHasher128(const unsigned char * key,int keySize, unsigned char * resBuf);

#endif /* __DEHT_HASH_FUNCS_H__ */
