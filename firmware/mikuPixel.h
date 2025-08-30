#pragma once

#include "pico/stdlib.h"
#include <stdio.h>

#ifdef NDEBUG
#define DBG_PRINT(fmt, ...)
#define DBG_PRINT_NA(fmt)
#define DBG_PUT(str)
#else


#define DBG_PRINT(fmt, ...) printf(fmt, __VA_ARGS__)
#define DBG_PRINT_NA(fmt) printf(fmt)
#define DBG_PUT(str) puts(str)
#endif

extern char macAddress[13];
