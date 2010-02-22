
#ifndef __UTIL_H__
#define __UTIL_H__


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


#endif /* __UTIL_H__ */
