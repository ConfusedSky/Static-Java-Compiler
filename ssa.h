#ifndef SSA_HEADER
#define SSA_HEADER

#include "cstructures.h"

typedef enum _operandType
{
    SSA_None,
    SSA_Variable,
    SSA_Value,
    SSA_Offset,
    SSA_Label
} SSAOperandType;

typedef struct _ssa_operand
{
   SSAOperandType tag;
   union
   {
       struct
       {
            char variableType;
            char variableNumber; 
       };
       int value;
       int offset;
       const char * label;
   };
} SSAOperand;

typedef struct _ssa_instruction
{
    const char * name;
    SSAOperand left, right1, right2; 

} SSAInstruction;

typedef struct _ssa_variable
{
    //const char * name;
    int count;
} SSAVariable;

typedef struct _ssa_program
{
    Class * class;
    SSAInstruction code[256];
    int code_count;
    SSAVariable variables[256];
    int variable_count;
    int tempCount;
    int tempStack[256];
    int tempStackHeight;
} SSAProgram;

int generateSSA(SSAProgram *, Class *); 
void printSSA(SSAProgram *); 
#endif
