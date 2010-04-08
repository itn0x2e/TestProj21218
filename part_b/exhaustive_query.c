#include "../common/ui.h"
#include "../common/utils.h"
#include "../DEHT/DEHT.h"

bool_t exhaustive_query(const char * prefix);
bool_t queryExaustiveTable(const char * prefix);

int main(int argc, char** argv) {
	if (2 != argc) {
		printUsage();
		return 1;
	}
	
	if (exhaustive_query(argv[1])) {
		return 0;
	}
	
	return 1;
}

bool_t exhaustive_query(const char * prefix) {
	DEHT* deht = NULL;
	CHECK(NULL != );
	CHECK(verifyDEHTExist(prefix));
	return queryExaustiveTable(prefix);
}