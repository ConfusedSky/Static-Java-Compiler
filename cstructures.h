#ifndef _CSTRUCTURES_GUARD
#define _CSTRUCTURES_GUARD

#include "types.h"

typedef struct _type
{
	TYPES type;
} Type;

typedef union _operand
{
	int value;
} Operand;

typedef struct _instruction
{
	int opCode;
	Operand operands[];
} Instruction;

typedef struct _method
{
	char name[100];
	short access_flags;	
} Method;

#endif
