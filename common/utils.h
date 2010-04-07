
#ifndef __UTILS_H__
#define __UTILS_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>

#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ERROR(msg) fprintf(stderr, "ERROR: %s:%d (%s): %s\n", __FILE__, __LINE__, __FUNCTION__, msg); \
			if (0 != errno) { \
		   		perror("possible error cause"); \
			}
#define WARN(msg) fprintf(stderr, "WARNING: %s:%d (%s): %s\n", __FILE__, __LINE__, __FUNCTION__, msg)

#ifdef DEBUG
	#define TRACE(msg) fprintf(stderr, "TRACE: %s:%d (%s): %s\n", __FILE__, __LINE__, __FUNCTION__, msg)
	#define TRACE_FPRINTF fprintf
#else
	#define TRACE(msg) /* In release - do nothing for trace messages */
	#define TRACE_FPRINTF sizeof
#endif /* DEBUG */

#define TRACE_FUNC_ENTRY() TRACE("entered")
#define TRACE_FUNC_EXIT() TRACE("exiting")
#define TRACE_FUNC_ERROR() TRACE("exiting due to error")

#define ASSERT(x) \
		assert(x);
		
#define FAIL(msg) \
		ERROR(msg); \
		goto LBL_ERROR;


#ifdef DEBUG
	#define CHECK(x) if (!(x)) {  \
				ERROR("Assertion failed: " #x); \
				goto LBL_ERROR; \
			 }
#else
	#define CHECK(x) if (!(x)) {  \
				/* silent in release */ \
				goto LBL_ERROR; \
			 }
#endif /* DEBUG */

#define FREE(x) \
		if(NULL != x) { \
			free(x); \
			x = NULL; \
		}

#define FCLOSE(x) \
		if(NULL != x) { \
			fflush(x); \
			fclose(x); \
			x = NULL; \
		}


/* wrapper for strncpy - assures null termination */
#define SAFE_STRNCPY(dst, src, total_size) \
	strncpy(dst, src, total_size - strlen(dst)); \
	dst[total_size - 1] = '\0'

/* wrapper for strncat - makes safe use of strncat bareable */
#define SAFE_STRNCAT(dst, src, total_size) \
	strncat(dst, src, total_size - strlen(dst)); \
	dst[total_size - 1] = '\0'




#endif /* __UTIL_H__ */
