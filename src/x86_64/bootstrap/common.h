#pragma once
#define BOOTSTRAP_CODE asm(".code32");

#ifndef __cplusplus
#define bool	_Bool
#define true	1
#define false	0
#endif

typedef signed long ssize_t;
typedef unsigned long size_t;

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
