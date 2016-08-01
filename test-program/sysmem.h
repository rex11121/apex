#ifndef SYS_MEM_H
#define SYS_MEM_H
#if defined(SYS_MEM)
#define EXTERN extern
#else
#define EXTERN
#endif

#include "systypes.h"
EXTERN int mem_fetch(u32_t,u32_t * restrict);
EXTERN int mem_write(u32_t,u32_t);
#endif
