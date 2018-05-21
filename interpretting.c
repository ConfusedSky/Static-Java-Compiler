#include "interpretting.h"
#include "bytecodes.h"
#include "scanning.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define STACK_MAX_SIZE 256
#define MAX_LOCAL_VARIABLES 256

#define DEBUG 0 
#define debugPrint if(DEBUG) printf

typedef struct _istack
{
	int data[STACK_MAX_SIZE];
	int loc;
} IntStack;

int stack_init(IntStack * stack)
{
	stack->loc = 0;
	return 0;
}

void stack_push(IntStack * stack, int value)
{
	if(stack->loc >= STACK_MAX_SIZE)
	{
		puts("Stack Overflow");
		exit(1);
	}
	stack->data[stack->loc++] = value;
}

int stack_pop(IntStack * stack)
{
	if(stack->loc <= 0)
	{
		puts("Nothing in stack");
		exit(1);
	}
	return stack->data[--stack->loc];
}

int stack_peek(IntStack * stack)
{
	if(stack->loc <= 0)
	{
		puts("Nothing in stack");
		exit(1);
	}
	return stack->data[stack->loc-1];
}

int stack_print(IntStack * stack)
{
	printf("Stack size: %i\n", stack->loc);

	for(int i = 0; i < stack->loc; i++)
	{
		printf("\t%i\n", stack->data[i]);
	}
	puts("");
}

typedef struct _context
{
	Class * class;
	IntStack stack;
	int variables[MAX_LOCAL_VARIABLES];
	int hasReturn;
} Context;

int initializeContext(Context * context, Class * class)
{
	context->class = class;
	stack_init(&(context->stack));
	context->hasReturn = 0;
	return 0;
}

int interpretMethod(Context *, Method *);

int callMethod(Context * c, CPInfo * constant)
{
	CPInfo * class = &(c->class->constant_pool[constant->class_index]);
	char * className = derefConstant(c->class->constant_pool, class->name_index);
	CPInfo * name = &(c->class->constant_pool[constant->name_and_type_index]);
	char * methodName = derefConstant(c->class->constant_pool, name->name_index);
	
	if(strcmp(className, c->class->name) == 0)
	{
		int methodNotFound = 1;

		for(int i = 0; i < c->class->method_count; i++)
		{
			Method * method = &(c->class->methods[i]);

			if(strcmp(methodName, method->name) == 0)
			{
				methodNotFound = 0;

				Context context;
				Context * con = &context;
				initializeContext(con, c->class);

				for(int i = method->parameter_count-1; i >= 0; i--)
				{
					con->variables[i] = stack_pop(&(c->stack));
				}

				interpretMethod(con, method);

				if(con->hasReturn)
				{
					stack_push(&(c->stack), stack_pop(&(con->stack)));
				}
				
				break;
			}
		}

		if(methodNotFound)
		{
			printf("%s: not found in this class\n", methodName);
			exit(1);
		}
	}
	else if(strcmp(methodName, "println") == 0)
	{
		printf("%i\n", stack_pop(&(c->stack)));
	}
	else
	{
		puts("Invalid method");
		exit(1);
	}
	
	return 0;
}

int interpretInstruction(Context * c, Instruction * i)
{
	int left;
	int right;

	if(DEBUG) stack_print(&c->stack);
	debugPrint("%s\n\n", getBytecodeName(i->op_code));

	switch(i->op_code)
	{
		case 0x03: //iconst_0
			stack_push(&c->stack, 0);
			break;
		case 0x10: //bipush
			stack_push(&c->stack, i->operands[0].value);
			break;
		case 0x1a: //iload_0
			stack_push(&c->stack, c->variables[0]);
			break;	
		case 0x1b: //iload_1
			stack_push(&c->stack, c->variables[1]);
			break;
		case 0x3c: //istore_1
			left = stack_pop(&c->stack);
			c->variables[1] = left;
			break;
		case 0xac: //ireturn
			c->hasReturn = 1;
			return 1;
			break;
		case 0xb1: //return
			return 1;
			break;
		case 0xb2: //getstatic
			// This is ignored because at the moment the only one we use is System.out
			break;
		case 0xb6: //invokevirtual
			// This is treated the same was as invoke static 
			// because we aren't worrying about objects yet.
		case 0xb8: //invokestatic
			left = (i->operands[0].value << 8) + i->operands[1].value;
			callMethod(c, &(c->class->constant_pool[left]));
			break;
		default:
			printf("0x%02x:%s is not defined.\n", i->op_code, getBytecodeName(i->op_code));
			stack_print(&c->stack);
			exit(1);
	}
	return 0;
}

int interpretMethod(Context * con, Method * m)
{
	debugPrint("\nEnter %s:\n", m->name);
	Instruction * instruction = m->code;

	for(int i = 0; i < m->instruction_count; i++)
	{
		if(interpretInstruction(con, instruction))
			break;
		
		instruction++;
	}	

	if(DEBUG) stack_print(&con->stack);
	debugPrint("Exit %s:\n\n", m->name);

	return 0;
}

int interpret(Class * c)
{
	Context context;
	Context * con = &context;
	initializeContext(con, c);

	int mainFound = 1;

	Method * method = c->methods;
	for(int i = 0; i < c->method_count; i++)
	{
		if(strcmp(method->name, "main") == 0)
		{
			mainFound = 0;
			interpretMethod(con, method);
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
