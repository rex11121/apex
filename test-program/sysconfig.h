#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

#define MEM_SIZE 10000
#define INSTRUCTION_CACHE_SIZE 200000
#define INSTRUCTION_START_ADDRESS 20000
#define INSTRUCTION_BUFFER_SIZE 150
#define NUM_OF_REGS 9
#define NUM_OF_STAGES 5
#include "systypes.h"

 //this number is computed by perfect_hashing (source file:perfect_hashing.c)
 // Once the environment is changed,this number should also change
#define PERFECT_HASHING_FACTOR 323630
#endif
