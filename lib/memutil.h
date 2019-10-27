#include <stdlib.h>
#include <stdio.h>

#define ERROR_ALLOC_MSG "Err: Insufficient memory."

#define VAL_TO_HEAP(ptr, _type, data) \
do \
{ \
  	if ( !( ptr = (_type *) malloc(sizeof(_type) * (1))) ) \
  	{ \
    		fprintf(stderr, ERROR_ALLOC_MSG); \
    		exit(EXIT_FAILURE); \
  	} \
  	*ptr = data; \
} while(0); \
(void *) ptr;
