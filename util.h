
#ifndef __UTIL_H__
#define __UTIL_H__

#define ERROR(x) fprintf(stderr, "ERROR: %s: %s", __FUNCTION__, x)
#define WARN(x) fprintf(stderr, "WARNING: %s: %s", __FUNCTION__, x)
#define TRACE(x) fprintf(stderr, "TRACE: %s: %s", __FUNCTION__, x)


#define CHECK(x) if (!(x)) {  \
			ERROR("Assertion failed: #x"); \
			goto LBL_ERROR; \
		 }


#endif /* __UTIL_H__ */
