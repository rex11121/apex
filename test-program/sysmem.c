/*
Author: Jiaming Zhang
implement memory access operations
 */
#define SYS_MEM
#include "sysconfig.h"
#include "sysmem.h"
#include"debug.h"
#include <stdio.h>
static u32_t mem[MEM_SIZE]={0};

int mem_fetch(u32_t addr,u32_t *restrict pdata )
{
#ifdef _DEBUG
	  if(addr>=MEM_SIZE)
	  {
		    APEX_TRACE("warning:memory address 0x%08x is greater than %d.It will wrap around!\n",addr,MEM_SIZE);
	  }
#endif
	  addr %= MEM_SIZE;
	  *pdata = mem[addr];
	  return 0;
}
int mem_write(u32_t addr,u32_t data)
{
#ifdef _DEBUG
	  if(addr>=MEM_SIZE)
	  {
		    APEX_TRACE("warning:memory address 0x%08x is greater than %d.It will wrap around!\n",addr,MEM_SIZE);
	  }
#endif
	  addr%=MEM_SIZE;
	  mem[addr] = data;
	  return 0;
}


