#include "interpretting.h"
#include "bytecodes.h"
#include <string.h>
#include <stdio.h>

typedef struct _context
{
	Class * class;
} Context;

int interpretInstruction(Context * c, Instruction * i)
{
	puts(getBytecodeName(i->op_code));
	return 0;
}

int interpretMethod(Context * c, Method * m)
{
	Instruction * instruction = m->code;

	for(int i = 0; i < m->instruction_count; i++)
	{
		interpretInstruction(c, instruction);
		instruction++;
	}	

	return 0;
}

int initializeContext(Context * context, Class * class)
{
	context->class = class;
	return 0;
}

int interpret(Class * c)
{
	Context context;
	initializeContext(&context, c);

	int mainFound = 1;

	Method * method = c->methods;
	for(int i = 0; i < c->method_count; i++)
	{
		if(strcmp(method->name, "main") == 0)
		{
			mainFound = 0;
			interpretMethod(&context, method);
			break;
		}

		method++;
	}

	if(mainFound)
	{
		puts("Main function not found!");
	}

	return mainFound;
}
