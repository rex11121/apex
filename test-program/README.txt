Description:
cpu.c : implements the pipeline of cpu.When the program is terminated by halt,the halt will be shown in the WB stage instead of a nop.
sysmem.c: implements the memory 
ic.c: implements the operation for instruction fetching and mapping
main.c: implements the command parsing

Implementation:
The details are documented in cpu.c.The dependencies are resovled by interlocking as is discussed in the class.When there is flow dependency,the pipeline will stall in D/RF stage.When executing a cycle,the pipeline starts processing from WB to Fetch stage and copy the result context to next stage until fetch stage or D/RF (if it stalls).

Note : when compiled by option -D _DEBUG,all instruction that does not support will be printed out and the cpu will not initialize the memory.
