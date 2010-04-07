#include "create_authentication_common.h"

int main(int argc, char ** argv) {
	if (3 != argc) {
		fprintf(stderr, "Error: Usage create_authentication <hash function name> <filename to create>\n");
		return 1;
	}

	/* return 0 if create_authentication returned successfully, otherwise 1 */
	if (create_authentication(argv[2], argv[1], FALSE)) {
		return 0;
	}

	return 1;
}
