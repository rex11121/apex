/*
   this file is not part of the project since it is used as a tool to facilitate the programming.
   It is used to map instruction operation string to binary from 0-14( try to find a minimal perfect hashing function)
   */

#include <stdio.h>
#include <string.h>
#define SIZE 15
const char * ins [] = {
"ADD","SUB","MOVC","MOV","MUL","AND","OR","EX-OR","LOAD","STORE",
"BZ","BNZ","JUMP","BAL","HALT"
};


unsigned int hash(const char * str,unsigned int para)
{
	  unsigned int s=0;
	  for(int i=0;str[i];++i)
		    s=s*para+str[i];
	  return s;
}
int marks[SIZE];

int check(unsigned para)
{
	  memset(marks,0,sizeof marks);
	  for(int i=0;i<sizeof ins/sizeof(char*);++i)
	  {
		    int s = hash(ins[i],para)%SIZE;
		    if(marks[s]) return 0;
		    marks[s] = 1;
	  }
	  return 1;
}
int main()
{
	  for(unsigned int i=1;;++i)
	  {//try to find an i such that hash()%SIZE could be a perfect hash function
		    if(check(i))
		    {
				printf("para should be %u\n",i);
				printf("perfect mapping:\n");
				for(int j=0;j<sizeof ins/sizeof(char*);++j)
				{
					  printf("%s->%u\n",ins[j],hash(ins[j],i)%SIZE);
				}
				return 0;
		    }

	  }
	  return 0;
}
