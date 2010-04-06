
#include <stdio.h>
#include "DEHT.h"
#include "utils.h"



int hashKeyIntoTableFunction(const unsigned char * key, int keySize, int tableSize)
{
	int cksum = 0;
	int i = 0;
	for (i = 0; i < keySize; ++i) {
		cksum = cksum ^ key[i];
	}

	return (cksum  % tableSize);
}

int hashKeyforEfficientComparisonFunction(const unsigned char * key,int keySize, unsigned char * resBuf)
{
	memcpy(resBuf, key, keySize);
}




bool_t createEmptyTable(void)
{
	bool_t ret = FALSE;
	
	DEHT * ht = NULL;

	ht = create_empty_DEHT("test", hashKeyIntoTableFunction, hashKeyforEfficientComparisonFunction, 
			       "test_dict",
				10, 5, 100);

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:

	if (NULL != ht) {
		lock_DEHT_files(ht);
		ht = NULL;
	}
	
	return ret;
}


bool_t openEmptyTable(void)
{
	bool_t ret = FALSE;
	
	DEHT * ht = NULL;

	ht = load_DEHT_from_files("test", hashKeyIntoTableFunction, hashKeyforEfficientComparisonFunction);

	ret = TRUE;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = FALSE;

LBL_CLEANUP:

	if (NULL != ht) {
		lock_DEHT_files(ht);
		ht = NULL;
	}
	
	return ret;
}



int main(void)
{
	int ret = -2;

	byte_t test_buf[] = "abcdef";

	FILE * fd = fopen("test.txt", "w+");
	printf("1 fwrite: %d\n", fwrite(test_buf, sizeof(test_buf), 1, fd));
	printf("1 fflush: %d\n", fflush(fd));

	printf("2 fseek: %d\n", fseek(fd, SEEK_SET, 0));
	perror("fseek error could be");
	printf("2 fwrite: %d\n", fwrite(test_buf, sizeof(test_buf), 1, fd));
	printf("2 fflush: %d\n", fflush(fd));

	printf("3 fseek: %d\n", fseek(fd, SEEK_SET, 0));
	perror("fseek error could be");
	printf("3 fwrite: %d\n", fwrite(test_buf, sizeof(test_buf), 1, fd));
	printf("3 fflush: %d\n", fflush(fd));

	fseek(fd, SEEK_END, 0);
	printf("\ntotal file size so far: %d\n", ftell(fd));

	printf("\n\n\n");

	printf("4 fseek: %d\n", fseek(fd, SEEK_SET, 5));
	perror("fseek error could be");
	printf("4 fwrite: %d\n", fwrite(test_buf, sizeof(test_buf), 1, fd));
	printf("4 fflush: %d\n", fflush(fd));

	printf("5 fseek: %d\n", fseek(fd, SEEK_SET, 6));
	perror("fseek error could be");
	printf("5 fwrite: %d\n", fwrite(test_buf, sizeof(test_buf), 1, fd));
	printf("5 fflush: %d\n", fflush(fd));

	printf("6 fseek: %d\n", fseek(fd, SEEK_SET, 7));
	perror("fseek error could be");
	printf("6 fwrite: %d\n", fwrite(test_buf, sizeof(test_buf), 1, fd));
	printf("6 fflush: %d\n", fflush(fd));

	fseek(fd, SEEK_END, 0);
	printf("\ntotal file size after all: %d\n", ftell(fd));

	fclose(fd);
	fd = NULL;

	printf("\n\n\n");

	CHECK(createEmptyTable());
	printf("createEmptyTable passed\n");
	CHECK(openEmptyTable());
	printf("openEmptyTable passed\n");

	printf("\ndone.\n");

	ret = 0;
	goto LBL_CLEANUP;

LBL_ERROR:
	TRACE_FUNC_ERROR();
	ret = -1;

LBL_CLEANUP:

	return ret;
}
