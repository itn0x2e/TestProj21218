#include "authenticate_common.h"

int main(int argc, char ** argv) {
	if (2 != argc) { 
		fprintf(stderr, "Error: Usage authenticate <authentication table text file>\n");
		return 1;
	}

	/* return 0 if authenticate returned successfully, otherwise 1 */
	if (authenticate(argv[1], FALSE)) {
		return 0;
	}
	
	return 1;
}
