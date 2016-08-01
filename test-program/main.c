/*
Author:Jiaming Zhang
Desc: Implementation of command line for the simulator.
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ic.h"
#include "sysconfig.h"
#include "cpu.h"

static char buffer[50];
static void run()
{
	  for(;;)
	  {
		    printf("apex>");
		    fflush(stdin);
		    fflush(stdout);
		    scanf("%s",buffer);
		    if(strcmp(buffer,"Initialize")==0)
		    {
				cpu_init();
				printf("cpu has been initialized \n");
		    }
		    else if(strcmp(buffer,"Display")==0)
		    {
				cpu_state_print();
		    }
		    else if(strcmp(buffer,"Simulate")==0)
		    {
				u32_t cycle;
				if(scanf(" %u",&cycle)!=1)
				{
					  fprintf(stderr,"format should be Simulate <n>\n");
					  exit(1);
				}
				cpu_exe(cycle);
				printf("%u cycle%s executed\n",cycle,cycle<=1?"\0":"s");
		    }
		    else exit(1);
	  }
}
int main(int argc,char **argv)
{
	  if(argc!=2)
	  {
		    fprintf(stderr,"usage: apex fileName\n");
		    exit(1);
	  }
	  if(cache_init(argv[1])<0)
	  {
		    fprintf(stderr,"can not open the file %s\n",argv[1]);
		    exit(1);
	  }
	  cpu_init();
	  run();
	  return 0;
}
