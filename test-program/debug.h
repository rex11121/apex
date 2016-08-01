#ifndef DEBUG_H
#define DEBUG_H
#ifdef _DEBUG
#define APEX_TRACE(...) do {\
	  printf("Trace information at line %d in file %s:",__LINE__,__FILE__);\
	  printf(__VA_ARGS__);}while(0)
#define sscanf_safe(n,l,...) do {\
			if(sscanf(__VA_ARGS__) !=n) \
	  			fprintf(stderr,"warning%d:wrong instruction format at line %d in the test file\n",__LINE__,l+1);\
}while(0)

#else 
#define APEX_TRACE(...)
#define sscanf_safe(n,l,...) sscanf(__VA_ARGS__)
#endif

#endif
