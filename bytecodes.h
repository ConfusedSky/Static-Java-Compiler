#ifndef BYTESCODES_HEADER
#define BYTESCODES_HEADER

typedef struct _opcode_info
{
	const char * name;
	int operand_count;
} BytecodeInfo;

const BytecodeInfo * getBytecodeInfo(unsigned char bytecode);
int getBytecodeOperandCount(unsigned char bytecode);
const char * getBytecodeName(unsigned char bytecode);

#endif
