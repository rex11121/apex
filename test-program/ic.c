/*
Author:Jiaming Zhang
Desc:the implementation uses offset from the start of the file to denote each instruction.
*/
#include "sysconfig.h"
#include "systypes.h"
#include "debug.h"
#include <stdio.h>
#include <ctype.h>
static u32_t cache[INSTRUCTION_CACHE_SIZE];
static u32_t valid_size;
static FILE * testFile;

#ifdef _DEBUG
#define ADDR_CHECK(addr) do{\
	  if(addr<INSTRUCTION_START_ADDRESS){\
		    APEX_TRACE("warning:the program tries to access memory address 0x%08x,which is lower than 0x4E20\n",addr);}\
}while(0)
#else
#define ADDR_CHECK(addr)
#endif

static int is_blank_line(const char * line)
{//if a line is blank it should be treated as nop.It is easy for finding mistakes in test files.

	  int cnt=0;
	  for(int i=0;line[i];++i)
	  {
		    if(isalnum(line[i])) ++cnt;
	  }
	  return !cnt;
}

int cache_init(const char *fileName)
{

	  if(!(testFile = fopen(fileName,"r")))
	  {
		    return -1;
	  }
	  for(valid_size=0;!feof(testFile);++valid_size)
	  {
		    char c;
		    cache[valid_size] = ftell(testFile);
		    while((c=fgetc(testFile))!='\n' && c!=EOF);
	  }
	  if(valid_size>=INSTRUCTION_CACHE_SIZE) return -1;
	  return 0;
}
int fetch_instruction(u32_t addr,char * pins)
{
	  if(testFile == NULL)
	  {
		    fprintf(stderr," cache should be initialized first !!!!!\n");
		    return -1;
	  }
	  ADDR_CHECK(addr);
	  addr-=INSTRUCTION_START_ADDRESS;
	  if(pins==NULL || addr>=valid_size) return -1;
	  fseek(testFile,cache[addr],SEEK_SET);
	  if(fgets(pins,INSTRUCTION_BUFFER_SIZE,testFile)==NULL||is_blank_line(pins)) return -1;
	  return 0;
}


