#ifndef _CSTRUCTURES_GUARD
#define _CSTRUCTURES_GUARD

#include "types.h"

#ifndef C_CODE_LENGTH
#define C_CODE_LENGTH 50
#endif

#ifndef C_MAX_PARAMETERS
#define C_MAX_PARAMETERS 6
#endif

typedef struct _type
{
	TYPES type;
	Type * subtype;
} Type;

typedef union _operand
{
	int value;
} Operand;

typedef struct _instruction
{
	int op_code;
	Operand operands[4];
} Instruction;

typedef struct _method
{
	char name[100];
	short access_flags;	
	Instruction code[C_CODE_LENGTH];
	Type return_type;	
	int parameter_count;
	Type parameter_types[C_MAX_PARAMETERS];
} Method;

typedef struct _field
{
	char name[100];
	short access_flags;
	Type type;
} Field;

typedef struct _class
{
	char name[100];
	short access_flags;
	struct _class * superclass;
	// Lets not worry about interfaces for now.
	int field_count;
	Field fields[10];
	int methods_count;
	Method methods[50];
} Class;

#endif
