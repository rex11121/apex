/**
Author:Jiaming Zhang
Desc: Implements the execution of CPU and each of its pipelines
   */

#include "sysconfig.h"
#include "ic.h"
#include "sysmem.h"
#include "cpu.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#define NUM_OF_INSTRUCTIONS 15
static u32_t regs[NUM_OF_REGS];//R0-R7(0-7) and X(8) register
static char  reg_valid_bits[NUM_OF_REGS];//bits for interlocking
static char ZF; // zero flag
static u32_t PC;//

struct  iformat // the structure used by pipeline.It is generated by decoded stage.
{
	  u32_t type;
	  u32_t dest;
	  u32_t src1;
	  u32_t src2;
	  i32_t literal;
	  u32_t result;//
};
static char ins_buffer[NUM_OF_STAGES][INSTRUCTION_BUFFER_SIZE]; // 
static char ins_valid[NUM_OF_STAGES];//valid bits for ins_buffer

static int allocate_ins_buffer()
{
	  int i=-1;
	  while(i<NUM_OF_STAGES && ins_valid[++i]);
	  assert(i<NUM_OF_STAGES);
	  ins_valid[i]=1;
	  return i;
}
static void free_ins_buffer(int idx)
{
	  ins_valid[idx] = 0;
}

struct stage_content
{
	  u32_t pc;// address of the current instruction
	  i32_t ins; // pointer pointing to the ascii format instruction
	  struct iformat * decoded;//decoded instruction info.
};

static struct stage_content stage_context[NUM_OF_STAGES]; 




/****************		instructions	********************/
/*
op->type
ADD->13
SUB->2
MOVC->1
MOV->4
MUL->9
AND->3
OR->12
EX-OR->5
LOAD->6
STORE->10
BZ->0
BNZ->11
JUMP->14
BAL->7
HALT->8
 */
static void squash()
{
	  if(stage_context[0].ins >=0)
	  {
		    free_ins_buffer(stage_context[0].ins);
		    stage_context[0].ins=-1;
	  }
	  if(stage_context[1].ins>=0)
	  {
		    free_ins_buffer(stage_context[1].ins);
		    stage_context[1].ins=-1;
	  }
}
static void add()
{
	  struct iformat * pf = stage_context[2].decoded;
	  if((pf->result = pf->src1+pf->src2)==0)
	  {
		    ZF=1;
	  }
	  else ZF=0;
}
static void sub()
{
	  struct iformat * pf = stage_context[2].decoded;
	  if((pf->result = pf->src1-pf->src2)==0)
	  {
		    ZF=1;
	  }
	  else ZF=0;

}
static void movc()
{
	  struct iformat * pf = stage_context[2].decoded;
	  pf->result = pf->literal;

}
static void mov(){assert(0);} //it is implemented by add instruction.It should not be executed in this way.
static void mul()
{
	  struct iformat * pf = stage_context[2].decoded;
	  if((pf->result = pf->src1*pf->src2)==0)
	  {
		    ZF=1;
	  }
	  else ZF=0;
}
static void and()
{
	  struct iformat * pf = stage_context[2].decoded;
	  if((pf->result = pf->src1&pf->src2)==0)
	  {
		    ZF=1;
	  }
	  else ZF=0;
}
static void or()
{
	  struct iformat * pf = stage_context[2].decoded;
	  if((pf->result = pf->src1|pf->src2)==0)
	  {
		    ZF=1;
	  }
	  else ZF=0;
}
static void xor()
{
	  struct iformat * pf = stage_context[2].decoded;
	  if((pf->result = pf->src1^pf->src2)==0)
	  {
		    ZF=1;
	  }
	  else ZF=0;
}
static void load()
{
	  struct iformat * pf = stage_context[2].decoded;
        pf->result = pf->src1+pf->literal;//literal could be negative and it does not affect the result
}
static void store()
{
	  struct iformat * pf = stage_context[2].decoded;
        pf->result = pf->src2+pf->literal;//literal could be negative and it does not affect the result
}
static void bz()
{
	  if(ZF)
	  {//branch is taken
		    //squash the instructions in fetch stage and D/RF stage by turning them into nop
		    assert( stage_context[1].decoded==NULL);
		    squash();
		    PC = stage_context[2].pc+stage_context[2].decoded->literal;
	  }
}
static void bnz()
{
	  if(ZF==0)
	  {
		    ZF=1;
		    bz();
		    ZF=0;
	  }
}
static void jump()
{ 
	  assert(stage_context[1].decoded==NULL);
	  squash();
	  PC=stage_context[2].decoded->literal +  stage_context[2].decoded->src1;
}
static void bal()
{ 
	  assert(stage_context[1].decoded==NULL);
	  squash();
	  regs[8] = stage_context[2].pc+1;//save return address
	  reg_valid_bits[8]=1;
	  PC=stage_context[2].decoded->literal +  stage_context[2].decoded->src1;
}
static void halt(){}//should not do anything
static void (*exec_op[])() = 
{
	  &bz,&movc,&sub,&and,&mov,&xor,&load,&bal,&halt,&mul,&store,&bnz,&or,&add,&jump
};

/*
   0--binary operation
   1- branch
   2-mov
   3-load,store
   4-halt
   5-branch and link
*/
static const u8_t ins_pro[15] = { [0]=1,[1]=2,[4]=2,[6]=3,[7]=5,[8]=4,[10]=3,[11]=1,[14]=5};







/************	functions for each state	************************/

static int exec_1()
{ // fetch stage
  //it should return -1 to terminate a cycle
	  stage_context[1] = stage_context[0]; ///move the data to next stage
	  stage_context[0].pc = PC;
	  stage_context[0].ins = allocate_ins_buffer();
	  stage_context[0].decoded = NULL; // no decoded information
	  if(fetch_instruction(PC,ins_buffer[stage_context[0].ins])==-1)
	  {///error in fetching -- turn it into a nop
		    free_ins_buffer(stage_context[0].ins);
		    stage_context[0].ins = -1;
		   return -1;
	  }
	  ++PC;
	  return -1;
}
static struct iformat * decode( const char * ins,u32_t addr)
{//decode the instruction
 //X register should be rewritten to R8 in ins

	  u32_t type=0;
	  int i=0;
	  int j;
	  while(isspace(ins[i])) ++i;
	  while(!isspace(ins[i]))
	  {
		  printf("%x ",(unsigned char )ins[i]);
		    type = type*PERFECT_HASHING_FACTOR+ins[i];
		    ++i;
	  }
	  
	  type%=NUM_OF_INSTRUCTIONS;
	  printf("type=%d\n",type);
	  if(ins_pro[type]==0)
	  {//Op dest src1 src2
		    int src1,src2,dest;
		    printf(".");
		    sscanf_safe(3,addr,ins+i," R%d R%d R%d",&dest,&src1,&src2);
		    if(!reg_valid_bits[dest]  || !reg_valid_bits[src1] || !reg_valid_bits[src2])
		    {
				return NULL;
		    }
		    struct iformat * pnew = malloc(sizeof (struct iformat));
		    pnew->type = type;
		    pnew->dest=dest;
		    pnew->src1=regs[src1];
		    pnew->src2=regs[src2];
		    reg_valid_bits[dest]=0;
		    return pnew;
	  }
	  if(ins_pro[type]==1)
	  {//bz liberal
		    struct iformat * pnew = malloc(sizeof (struct iformat));
		    pnew->type=type;
		    sscanf_safe(1,addr,ins+i,"%d",&pnew->literal);
		    return pnew;
	  }
	  if(ins_pro[type]==2)
	  {//movc,mov
		    int dest;
		    sscanf_safe(1,addr,ins+i," R%d",&dest);
		    if(reg_valid_bits[dest]==0) return NULL;
		    if(type==1)
		    {//movc
				struct iformat * pnew = malloc(sizeof (struct iformat));
				pnew->type=type;
				pnew->dest=dest;
				sscanf_safe(1,addr,ins+i," R%*d %d",&pnew->literal);
				reg_valid_bits[dest]=0;
				return pnew;
		    }
		    //mov dest,src ==> add dest,src,#0
		    int src;
		    sscanf_safe(1,addr,ins+i," R%*d R%d",&src);
		    if(reg_valid_bits[src]==0) return NULL;
		    reg_valid_bits[dest]=0;
		    struct iformat * pnew = malloc(sizeof (struct iformat));
		    pnew->type=13;
		    pnew->dest=dest;
		    pnew->src1=regs[src];
		    pnew->src2=0;
		    return pnew;
	  }
	  if(ins_pro[type]==3)
	  {//load r r lit or load r r r
		    int r1,r2,literal;
		    int r3,flag=0;
		    if(sscanf(ins+i," R%d R%d R%d",&r1,&r2,&r3) !=3)
		    {
				flag=1;
		    		sscanf_safe(3,addr,ins+i," R%d R%d %d",&r1,&r2,&literal);
		    }
		    if((type!=6 &&!reg_valid_bits[r1]) || !reg_valid_bits[r2]||(!flag&&!reg_valid_bits[r3])) return NULL;
		    struct iformat * pnew = malloc(sizeof (struct iformat));
		    pnew->type = type;
		    pnew->literal=literal;
		    if(!flag) pnew->literal=regs[r3];///Modified
		    if(type==6)
		    {//load
				reg_valid_bits[r1] = 0;
				pnew->dest=r1;
				pnew->src1 = regs[r2];
		    }
		    else
		    {//store
				pnew->src1=regs[r1];
				pnew->src2=regs[r2];
		    }
		    return pnew;
	  }
	  if(ins_pro[type]==4)
	  {//halt
		    struct iformat * pnew = malloc(sizeof(struct iformat));
		    pnew->type=type;
		    printf("*");
		    return pnew;
	  }
	  if(ins_pro[type]==5)
	  {//branch link
		    int src,literal;
		    sscanf_safe(2,addr,ins+i," R%d %d",&src,&literal);
		    if((type==7 && reg_valid_bits[8]==0)|| reg_valid_bits[src]==0) return NULL;//R8 should be available for BAL
		    if(type==7)
		    {
				reg_valid_bits[8]=0;
		    }
		    struct iformat *pnew = malloc(sizeof(struct iformat));
		    pnew->type=type;
		    pnew->src1=regs[src];
		    pnew->literal=literal;
		    return pnew;
	  }
	  assert(0);//no reach
	  return NULL;
}
static char exec_2_buffer[INSTRUCTION_BUFFER_SIZE];
static void rename_reg(const char * ins)
{//rename X to R8
	  int sz=0;
	  for(int i=0;ins[i];++i)
	  {
		    if(i&&ins[i]=='X'&&isspace(ins[i-1]))
		    {
				exec_2_buffer[sz] = 'R';
				exec_2_buffer[sz+1]='8';
				sz+=2;
		    }
		    else exec_2_buffer[sz++]=ins[i];
	  }
	  exec_2_buffer[sz]=0;
}
static int exec_2()
{//decode stage.It may stall if returns -1
	  if(stage_context[1].ins<0)
	  {
		    stage_context[2].ins=-1;
		    return 0; // nop instruction
	  }
	  if(exec_2_buffer[0]==0)
		    rename_reg(ins_buffer[stage_context[1].ins]);
	  if((stage_context[1].decoded = decode(exec_2_buffer,stage_context[1].pc-INSTRUCTION_START_ADDRESS))!=NULL)
	  {
		    stage_context[2] = stage_context[1];
		    exec_2_buffer[0]=0;
		    return 0;
	  }
	  stage_context[2].ins=-1;// the next stage 3 should be perform nop
	  return -1;//stall
}
static int exec_3()
{//execute stage
	  if(stage_context[2].ins>=0)
	  {
		    assert(stage_context[2].decoded!=NULL);// must be decoded
		    exec_op[stage_context[2].decoded->type]();//execute the instruction
	  }
	  stage_context[3] = stage_context[2];
	  return 0;
}
static int exec_4()
{//Mem stage
	  if(stage_context[3].ins<0) goto end;//nop
	  assert(stage_context[3].decoded != NULL);
	  if(stage_context[3].decoded->type == 6)
	  {//Load.
		//Note:there is no forwarding.Thus the result should be written back in WB
            //decoded->result should contain the address.
		if(mem_fetch(stage_context[3].decoded->result,&stage_context[3].decoded->result)<0)
		{
			  fprintf(stderr,"invalid access to memory address %u",stage_context[3].decoded->result);
			  exit(1);
		}
	  }
	  else if(stage_context[3].decoded->type==10 )
	  {//Store
		    mem_write(stage_context[3].decoded->result,stage_context[3].decoded->src1);
	  }
end:
	  stage_context[4] = stage_context[3];
	  return 0;
}
static void exit_program();
static int exec_5()
{//WB stage
	  int ins = stage_context[4].ins;
	  u32_t type=-1;
	  if(ins>=0)
	  {
		    assert(stage_context[4].decoded!=NULL);
		    type=stage_context[4].decoded->type;
		    if(type==8)
		    {//halt. terminate the program!
				exit_program();
				assert(0);//no return!!!!
		    }
		    if(ins_pro[type]==0 || ins_pro[type]==2 ||( ins_pro[type]==3 && stage_context[4].decoded->type!=10) )
		    {
				regs[stage_context[4].decoded->dest]=stage_context[4].decoded->result;
				reg_valid_bits[stage_context[4].decoded->dest] = 1;
		    }
		    free_ins_buffer(ins);
		    free(stage_context[4].decoded);
		    stage_context[4].ins=-1;
		    stage_context[4].decoded=NULL;
	  }
	  return 0;
}
static int (*stage_exe[5])() = {&exec_1,&exec_2,&exec_3,&exec_4,&exec_5};

static void execute()
{//execute for a cycle
	  int i=4;
	  while(stage_exe[i]()>=0)
	  {
		    --i;
	  }
}
void exit_program()
{
	  cpu_state_print();
	  exit(0);
}

/**********		exported functions	************/
void cpu_init()
{//initialzie env 
	  PC = INSTRUCTION_START_ADDRESS; //
	  for(int i=0;i<NUM_OF_REGS;++i)
	  {
		    reg_valid_bits[i]=1;//all register values should be valid before execution
	  }
	  for(int i=0;i<NUM_OF_STAGES;++i)
	  {
		    ins_valid[i]=0;//all the slots for instruction buffer used by each stage should be available
	          stage_context[i].ins=-1;//all stages contain a nop intruction before execution
	  }
	  exec_2_buffer[0]=0;//trivial.
}

void cpu_exe(u32_t cycles)
{//execute for cycles
	 while(cycles--)
	 {
		   execute();//may not return if halt is in WB stage.
	 }
}

void cpu_state_print()
{//print the state of cpu and contents in memory
     printf("Registers:\n");
     for(int i=0;i<NUM_OF_REGS-1;++i)
     {
		 printf("R%d=%d(%d) ",i,regs[i],reg_valid_bits[i]);
     }
     printf("X=0x%08x(%d) PC=0x%08x\n",regs[NUM_OF_REGS-1],reg_valid_bits[8],PC);
     static const char * name[] ={"Fetch","D/RF","EXE","MEM","WB"};
     printf("content of each stage:\n");
     for(int i=0;i<NUM_OF_STAGES;++i)
     {
		 printf("%s:\t",name[i]);
		 if(stage_context[i].ins==-1)
		 {
			   printf("NOP\n");
		 }
		 else
		 {
			   printf("%s",ins_buffer[stage_context[i].ins]);
			   printf("PC=0x%x\n",stage_context[i].pc);
		 }
     }
     printf("Memory contexts from 0-99:");
     for(int i=0;i<100;)
     {
		 if(i%16==0)
		 {
			   printf("\n0x%-5x:",i);
		 }
		 int j=0;
		 while(i+j<100 && j<16)
		 {
			   u32_t con;
			   mem_fetch(i+j,&con);
			   printf("%08x ",con);
			   ++j;
		 }
		 i+=j;
     }
     puts("");
}


