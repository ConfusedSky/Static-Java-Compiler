#include "ssa.h"
#include "util.h"
#include "bytecodes.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "scanning.h"

SSAOperand generateTemp(SSAProgram * ssap)
{
    SSAOperand temp;
    temp.tag = SSA_Variable;
    temp.variableType = 't';
    temp.variableNumber = ssap->tempCount++;
    ssap->tempStack[ssap->tempStackHeight] = temp.variableNumber;
    ssap->tempStackHeight++;
    return temp;
}

SSAOperand popTemp(SSAProgram * ssap)
{
    SSAOperand temp;
    temp.tag = SSA_Variable;
    temp.variableType = 't';
    ssap->tempStackHeight--;
    temp.variableNumber = ssap->tempStack[ssap->tempStackHeight];
    return temp; 
}

SSAOperand peekVar(SSAProgram * ssap, int i)
{
    SSAOperand temp;
    temp.tag = SSA_Variable;
    temp.variableType = i + 'a';
    if(ssap->variables[i].count < 1) ssap->variables[i].count = 1;
    temp.variableNumber = ssap->variables[i].count - 1;
    return temp; 
}

SSAOperand nextVar(SSAProgram * ssap, int i)
{
    ssap->variables[i].count++;
    return peekVar(ssap, i);
}

void initProgram(SSAProgram * program, Class * class)
{
    program->code_count = 0;
    program->class = class;
    program->tempCount = 0;
    program->tempStackHeight = 0;
}

void call(SSAProgram * ssap, SSAInstruction * instruction, int constantNo)
{
    CPInfo * constant = &(ssap->class->constant_pool[constantNo]);
    CPInfo * name = &(ssap->class->constant_pool[
        constant->name_and_type_index]);

    const char * functionName = derefConstant(
        ssap->class->constant_pool,
        name->name_index);

    Method * method = ssap->class->methods;

    for(int i = 0; i < ssap->class->method_count; i++)
    {
        if(strcmp(method->name, functionName) == 0)
        {
            break;
        }
        method++;
    }

    if(strcmp(functionName, "printInt") == 0)
    {
        instruction->name = "printInt";
    }
    else
    {
        for(int i = 0; i < method->parameter_count; i++)
        {
            popTemp(ssap);
        }
        instruction->name = "call";
        instruction->right1.tag = SSA_Label;
        instruction->right1.label = method->name;
        if(method->return_type.type != TYPES_VOID) 
            instruction->left = generateTemp(ssap);
    }
}

void generateProgram(SSAProgram * ssap, Method * method)
{
    SSAInstruction * instruction = &(ssap->code[ssap->code_count]);
    instruction->name = "label";
    instruction->left.tag = SSA_Label;
    instruction->left.label = method->name;
    instruction++;
    ssap->code_count++;
    for(int i = 0; i < method->instruction_count; i++)
    {
        switch(method->code[i].op_code)
        {
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x08:
            case 0x10:
                instruction->right1.tag = SSA_Value; 
                switch(method->code[i].op_code)
                {
                    case 0x03: //iconst_0
                        instruction->right1.value = 0;
                        break;
                    case 0x04: //iconst_1
                        instruction->right1.value = 1;
                        break;
                    case 0x05: //iconst_2
                        instruction->right1.value = 2;
                        break;
                    case 0x08: //iconst_5
                        instruction->right1.value = 5;
                        break;
                    case 0x10: //bipush
                        instruction->right1.value = 
                            method->code[i].operands[0].value;
                        break;
                }
                instruction->right2.tag = SSA_None;
                instruction->left = generateTemp(ssap);
                instruction->name = "assignConst";
                break;
            case 0x1a:
            case 0x1b: 
            case 0x1c:
            case 0x1d:
                switch(method->code[i].op_code)
                {
                    case 0x1a: // iload_0
                        instruction->right1 = peekVar(ssap, 0);
                        break;
                    case 0x1b: // iload_1 
                        instruction->right1 = peekVar(ssap, 1);
                        break;
                    case 0x1c: // iload_2
                        instruction->right1 = peekVar(ssap, 2);
                        break;
                    case 0x1d: // iload_3
                        instruction->right1 = peekVar(ssap, 3);
                        break;
                }
                instruction->left = generateTemp(ssap);
                instruction->right2.tag = SSA_None;
                instruction->name = "read";
                break; 
            case 0x3c: 
            case 0x3d:
            case 0x3e:
                switch(method->code[i].op_code)
                {
                    case 0x3c: // istore_1 
                        instruction->left = nextVar(ssap, 1);
                        break;
                    case 0x3d: // istore_2
                        instruction->left = nextVar(ssap, 2);
                        break;
                    case 0x3e: // istore_3
                        instruction->left = nextVar(ssap, 3);
                        break;
                }
                instruction->right1 = popTemp(ssap);
                instruction->right2.tag = SSA_None;
                instruction->name = "write";
                break; 
            case 0x60: //iadd
            case 0x64: //isub
                instruction->right2 = popTemp(ssap);
                instruction->right1 = popTemp(ssap);
                instruction->left = generateTemp(ssap);
                switch(method->code[i].op_code)
                {
                    case 0x60: //iadd
                        instruction->name = "add";
                        break;
                    case 0x64: //isub
                        instruction->name = "sub";
                        break;
                }
                break;
            case 0x84: //iinc
                instruction->right1 = 
                    peekVar(ssap, method->code[i].operands[0].value);
                instruction->right2.tag = SSA_Value;
                instruction->right2.value = (char) (0xFF & 
                    method->code[i].operands[1].value);
                instruction->left = 
                    nextVar(ssap, method->code[i].operands[0].value);
                instruction->name = "increment";
                break;
            case 0x9a: // ifne
            case 0x9e: // ifle
                instruction->right2.tag = SSA_Value;
                instruction->right2.value = 0;
                instruction->right1 = popTemp(ssap);
                switch(method->code[i].op_code)
                {
                    case 0x9a: // ifne
                        instruction->name = "branchNE";
                        break;
                    case 0x9e: // ifle
                        instruction->name = "branchLE";
                        break;
                }
                break;
            case 0xa0: //if_icmne
            case 0xa2: //if_icmge
                instruction->right2 = popTemp(ssap);
                instruction->right1 = popTemp(ssap);
                switch(method->code[i].op_code)
                {
                    case 0xa0: // if_icmne 
                        instruction->name = "branchNE";
                        break;
                    case 0xa2: // if_icmge 
                        instruction->name = "branchGE";
                        break;
                }
                break;
            case 0xa7: //goto
                instruction->name = "goto";
                break;
            case 0xac: //ireturn
                instruction->right1 = popTemp(ssap);
                instruction->name = "return";
                break;
            case 0xb1: //return
                instruction->name = "return";
                break;
            case 0xb8: //invokestatic
                call(ssap, instruction, (method->code[i].operands[0].value << 8) + 
                        method->code[i].operands[1].value);
                break;
            default:
                printf("0x%02x: %s is not defined.\n", 
                        method->code[i].op_code, 
                        getBytecodeName(method->code[i].op_code));
                printSSA(ssap);
                exit(1);
        }

        instruction++;
        ssap->code_count++;
    }
}

int generateSSA(SSAProgram * program, Class * class)
{
    initProgram(program, class);

    for(int i = 0; i < class->method_count; i++)
    {
        if(strcmp(class->methods[i].name, "<init>") != 0 &&
           strcmp(class->methods[i].name, "printInt") != 0)
        /*if(strcmp(class->methods[i].name, "main") == 0)*/
        {
            generateProgram(program, &(class->methods[i]));
        }
    }
    
    return 0;
}

void printOperand(SSAOperand operand)
{
    switch((int)operand.tag)
    {
        case SSA_None:
            printf("  ");
            break;
        case SSA_Variable:
            printf("%c%i", operand.variableType, operand.variableNumber);
            break;
        case SSA_Value:
            printf("%i", operand.value);
            break;
        case SSA_Offset:
            printf("%x", operand.offset);
            break;
        case SSA_Label:
            printf("%s", operand.label);
    }
}

void printInstruction(SSAInstruction instruction)
{
    if(strcmp(instruction.name, "label") == 0)
    {
        printf("%s:\n", instruction.left.label);
    }
    else
    {
        printf("\t");
        printOperand(instruction.left);
        printf(" = %s(", instruction.name);
        printOperand(instruction.right1);
        printf(", ");
        printOperand(instruction.right2);
        printf(")\n");
    }
}

void printSSA(SSAProgram * program)
{
    for(int i = 0; i < program->code_count; i++)
    {
        printInstruction(program->code[i]);
    }
}
