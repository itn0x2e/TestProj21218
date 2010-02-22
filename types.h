
#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned char byte;
typedef unsigned long ulong;
typedef unsigned long long ulonglong;

typedef enum bool_e {
	FALSE = 0,
	TRUE
} bool;

typedef enum algorithmId_e {
	ALGO_INVALID = 0,
	ALGO_SHA1,
	ALGO_MD5
} algorithmId;

#endif /* __TYPES_H__ */
