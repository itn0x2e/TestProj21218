#ifndef __CREATE_AUTHENTICATION_COMMON_H__
#define __CREATE_AUTHENTICATION_COMMON_H__

#include <stdio.h>
#include "../common/types.h"

bool_t create_authentication(char * filename, const char * hashFuncName, bool_t salty);

#endif /* __CREATE_AUTHENTICATION_COMMON_H__ */
