#ifndef IC_H
#define IC_H
#include "systypes.h"
#include "sysconfig.h"
int fetch_instruction(u32_t, char*);
int cache_init(const char *);
#endif
