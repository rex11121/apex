#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;

mt19937 eng(random_device{}());
uniform_int_distribution<int> dis_reg(0,7);
char buffer[300];
vector<int> procedures;
vector<bool> masks(8,false);
int free_count=8;
int get_free()
{
	int r = dis_reg(eng);
	if(free_count==0) return -1;
	while(masks[r]) r= dis_reg(eng);
	return r;
}
int get_free_s()
{
	int r;
	while((r=get_free()) == 6);
	return r;
}
string add()
{	if(free_count==0) return "";
	sprintf(buffer,"ADD R%d R%d R%d\n",get_free(),dis_reg(eng),dis_reg(eng));
	return string(buffer);
}
string sub()
{
	if(free_count==0) return "";
	sprintf(buffer,"SUB R%d R%d R%d\n",get_free(),dis_reg(eng),dis_reg(eng));
	return string(buffer);
}
string movc()
{
	if(free_count==0) return "";
	sprintf(buffer,"MOVC R%d %d\n",get_free(),uniform_int_distribution<>{}(eng));
	return string(buffer);
}
string mul()
{
	if(free_count==0) return "";
	sprintf(buffer,"MUL R%d R%d R%d\n",get_free(),dis_reg(eng),dis_reg(eng));
	return string(buffer);
}
string And()
{
	sprintf(buffer,"AND R%d R%d R%d\n",get_free(),dis_reg(eng),dis_reg(eng));
	return string(buffer);
}
string Or()
{
	sprintf(buffer,"OR R%d R%d R%d\n",get_free(),dis_reg(eng),dis_reg(eng));
	return string(buffer);
}
string Xor()
{
	sprintf(buffer,"EX-OR R%d R%d R%d\n",get_free(),dis_reg(eng),dis_reg(eng));
	return string(buffer);
}
string load()
{
	int ig = get_free();
	int lit = uniform_int_distribution<>{0,99}(eng);
	sprintf(buffer,"MOVC R%d %d\nLOAD R%d R%d %d\n",ig,lit
			,get_free(),ig,uniform_int_distribution<>{-lit,99-lit}(eng));
	return string(buffer);
}
string store()
{
	int lit = uniform_int_distribution<>{0,99}(eng);
	int rg=get_free();
	sprintf(buffer,"MOVC R%d %d\nSTORE R%d R%d %d\n",rg,lit,dis_reg(eng),rg,
			uniform_int_distribution<>{-lit,99-lit}(eng));
	return string(buffer);
}
string call_in_basic()
{
	int rg = get_free();
	int idx =uniform_int_distribution<>{0,(int)procedures.size()-1}(eng);
	int offset = procedures[idx];
	sprintf(buffer,"MOVC R%d %d\nBAL R%d 0\n",rg,offset,rg);
	return string(buffer);
}
string (* basic_ins[])() { &add,&sub,&mul,&movc,&And,&Or,&Xor,&load,&store,&call_in_basic};
int ins_size[]{1,1,1,1,1,1,1,2,2,2};
//string bz()
//{
//	return "BZ $     \n";
//}
//string bnz()
//{
//	return "BNZ $       \n";
//}
//basic flows
string basic_block(int env,int num,int &ins,int prob=1)
{
	string ret;
	ins=0;
	if(bernoulli_distribution(prob)(eng)==false) env=1;
	uniform_int_distribution<> dis(0,9-env);

 
	for(int i=0;i<num;++i)
	{
		int idx = dis(eng);
		ret+=basic_ins[idx]();
		ins+=ins_size[idx];
	}
	return ret;
}
string loop(int env,double &prob,int num,int &ins)
{
	ins=0;
	prob*=0.81;
	if(env) prob*=0.2;
	if(free_count<2) return "";
	string ret;
	int ig = get_free_s();
	masks[ig] =1;
	int tm = uniform_int_distribution<>{1,6}(eng);
	bernoulli_distribution ber(prob);
	for(int i=0;i<num;++i)
	{
		int in;
		if(ber(eng))
		{
			sprintf(buffer,"STORE R%d R7 0\nMOVC R6 1\nADD R7 R7 R6\n",
					ig);
			ret+=buffer;
			masks[ig]=0;
			free_count++;
			ins+=3;
			ret+=loop(env,prob,num>>1,in);
			sprintf(buffer,"MOVC R6 1\nSUB R7 R7 R6\nLOAD R%d R7 0\n",ig);
			ret+=buffer;
			ins+=3;
			masks[ig]=1;
			free_count--;
		}
		else
		{
			ret+=basic_block(env,uniform_int_distribution<>{8,28}(eng),in,prob*0.05);
		}
		ins+=in;
	}
	sprintf(buffer,"MOVC R%d %d\nEX-OR R6 R6 R6\nBZ %d\n",ig,tm,ins+1);
	ret = buffer+ret;
	sprintf(buffer,"MOVC R6 1\nSUB R%d R%d R6\nBNZ -%d\n",ig,ig,ins+2);
	ins+=6;
	ret+=buffer;
	masks[ig]=0;
	++free_count;
	return ret;
}
string blocks(int env,int num,int &ins)
{
	bernoulli_distribution ber(0.55);
	ins=0;
	string ret;
	for(int i=0;i<num;++i)
	{
		int in=0;
		if(ber(eng)&&(num>>1))
		{
			double d=1;
			ret+=loop(env,d,num>>1,in);
		}
		else
		{
			ret+=basic_block(env,uniform_int_distribution<>{2,17}(eng),in);
		}
		ins+=in;
	}
	return ret;
}
string procedure(int num,int &ins)
{
	ins=0;
	string ret = blocks(1,num,ins);
	sprintf(buffer,"JUMP X 0\n");
	++ins;
	ret+=buffer;
	return ret;
}
int main(int argc,char ** argv)
{
	int ins=3;
	int in;
	int nPro = (int)normal_distribution<double>(7,2)(eng);//number of procedures;
	masks[7]=1;
	free_count--;
	string pros;
	normal_distribution<> dis(6,2);
	for(int i=0;i<nPro;++i)
	{
		procedures.push_back(ins+20000);
		pros+=procedure((int)dis(eng),in);
		ins+=in;
	}
	string prefix = "MOVC R7 1000\nMOVC R6 20000\nJUMP R6 "+to_string(ins)+"\n";
	int nb = (int)normal_distribution<>(7,3)(eng);
	string ret;
	ret+=blocks(0,nb,in);
	ins+=in;
	++ins;
	cout<<prefix<<pros<<ret<<"HALT\n";
	return 0;
}
