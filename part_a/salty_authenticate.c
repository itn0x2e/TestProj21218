#include "authenticate_common.h"

int main(int argc, char ** argv) {
	if (2 != argc) { 
		fprintf(stderr, "Error: Usage salty_authenticate <authentication table text file>\n");
		return 1;
	}

	/* return 0 if authenticate returned successfully, otherwise 1 */
	if (authenticate(argv[1], TRUE)) {
		return 0;
	}
	
	return 1;
}
