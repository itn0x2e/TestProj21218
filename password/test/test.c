#include <stdio.h>
#include <string.h>
#include "AlphabetTest.h"
#include "DictionaryTest.h"
#include "PasswordGeneratorTest.h"

void hello() {
	byte_t test_buf[] = "abcdef";

	    FILE * fd = fopen("/tmp/test.txt", "w+");
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
}
int main(int argc, char** argv) {
	hello();
	int a, b;
	char s[20] = "1223-@";
	printf("%d", sscanf(s, "%d-%d", &a, &b));
	if (dictionaryTest()) {
	//if (alphabetTest()) {
	//if (generatorTest()) {
		printf("Test dictionary: SUCCESS\n");
	} else {
		printf("Test dictionary: FAILURE\n");
		return 1;
	}
	return 0;
}

