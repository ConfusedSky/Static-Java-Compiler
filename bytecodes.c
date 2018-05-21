#include "bytecodes.h"
#include <stdlib.h>
#include <stdio.h>

BytecodeInfo bytecodes[256];
int initialized = 0;
const char * none = "";

void addBytecode(int bytecode, const char * name, int operand_count)
{
	if(bytecodes[bytecode].name != none) 
	{
		printf("%i is already defined.\n", bytecode);
		exit(-1);
	}

	bytecodes[bytecode].name = name;
	bytecodes[bytecode].operand_count = operand_count;
}

void initialize()
{
	for(int i = 0; i < 256; i++)
	{
		bytecodes[i].name = none;
	}

	initialized = 1;

	addBytecode(  3, "iconst_0", 0);
	addBytecode(  4, "iconst_1", 0);
	addBytecode(  5, "iconst_2", 0);
	addBytecode(  6, "iconst_3", 0);
	addBytecode(  7, "iconst_4", 0);
	addBytecode(  8, "iconst_5", 0);
	addBytecode( 16, "bipush", 1);
	addBytecode( 18, "ldc", 1);
	addBytecode( 26, "iload_0", 0);
	addBytecode( 27, "iload_1", 0);
	addBytecode( 28, "iload_2", 0);
	addBytecode( 29, "iload_3", 0);
	addBytecode( 42, "aload_0", 0);
	addBytecode( 43, "aload_1", 0);
	addBytecode( 44, "aload_2", 0);
	addBytecode( 45, "aload_3", 0);
	addBytecode( 59, "istore_0", 0);
	addBytecode( 60, "istore_1", 0);
	addBytecode( 61, "istore_2", 0);
	addBytecode( 62, "istore_3", 0);
	addBytecode( 77, "astore_2", 0);
	addBytecode( 78, "astore_3", 0);
	addBytecode( 96, "iadd", 0);
	addBytecode(132, "iinc", 2);
	addBytecode(153, "ifeq", 2);
	addBytecode(154, "ifne", 2);
	addBytecode(155, "iflt", 2);
	addBytecode(156, "ifge", 2);
	addBytecode(157, "ifgt", 2);
	addBytecode(158, "ifle", 2);
	addBytecode(159, "if_icmeq", 2);
	addBytecode(160, "if_icmne", 2);
	addBytecode(161, "if_icmlt", 2);
	addBytecode(162, "if_icmge", 2);
	addBytecode(163, "if_icmgt", 2);
	addBytecode(164, "if_icmle", 2);
	addBytecode(167, "goto", 2);
	addBytecode(172, "ireturn", 0);
	addBytecode(177, "return", 0);
	addBytecode(183, "invokespecial", 2);
	addBytecode(178, "getstatic", 2);
	addBytecode(182, "invokevirtual", 2);
	addBytecode(184, "invokestatic", 2);
	addBytecode(191, "athrow", 0);
}

#define initializeOnce if(!initialized){ initialize(); }

const BytecodeInfo * getBytecodeInfo(unsigned char bytecode)
{
	initializeOnce
	if(bytecodes[bytecode].name == none)
	{
		printf("%i is not a defined bytecode\n", bytecode);
		exit(-1);
	}
	return &(bytecodes[bytecode]);
}

int getBytecodeOperandCount(unsigned char bytecode)
{
	return getBytecodeInfo(bytecode)->operand_count;
}

const char * getBytecodeName(unsigned char bytecode)
{
	return getBytecodeInfo(bytecode)->name;
}
