
#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <malloc.h>

#define MIN(a, b) ((a > b) ? b : a)
#define MAX(a, b) ((a > b) ? a : b)

#define ERROR(x) fprintf(stderr, "ERROR: %s: %s\n", __FUNCTION__, x)
#define WARN(x) fprintf(stderr, "WARNING: %s: %s\n", __FUNCTION__, x)
#define TRACE(x) fprintf(stderr, "TRACE: %s: %s\n", __FUNCTION__, x)

#define FAIL(x) \
		ERROR(x); \
		goto LBL_ERROR;


#define CHECK(x) if (!(x)) {  \
			ERROR("Assertion failed: " #x); \
			goto LBL_ERROR; \
		 }

#define FREE(x) \
		if(NULL != x) { \
			free(x); \
			x = NULL; \
		}

#define FCLOSE(x) \
		if(NULL != x) { \
			fclose(x); \
			x = NULL; \
		}


#endif /* __UTIL_H__ */
