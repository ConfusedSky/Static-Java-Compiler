#include "parsing.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>
#include "bytecodes.h"

// Char * so it is possible to read in the class type as well, if we ever get to it
int parseType(char * source, Type * t)
{
	switch( *source )
	{
		case 'B':
			t->type = TYPES_BYTE;
			break;
		case 'C':
			t->type = TYPES_CHAR;
			break;
		case 'D':
			t->type = TYPES_DOUBLE;
			break;
		case 'F':
			t->type = TYPES_FLOAT;
			break;
		case 'I':
			t->type = TYPES_INT;
			break;
		case 'J':
			t->type = TYPES_LONG;
			break;
		case 'L':
			t->type = TYPES_REFERENCE;
			break;
		case 'S':
			t->type = TYPES_SHORT;
			break;
		case 'Z':
			t->type = TYPES_BOOLEAN;
			break;
		case '[':
			t->type = TYPES_ARRAY;
			break;
		case 'V':
			t->type = TYPES_VOID;
			break;
		default:
			printf("Type(%c) not defined.\n", *source);
			exit(-1);
	}

	return 0;
}

int parseFields(Class * c, ClassInfo * ci)
{
	c->field_count = ci->fields_count;

	for(int i = 0; i < c->field_count; i++)
	{
		Field * field = &(c->fields[i]); 
		FieldInfo * fi = &(ci->fields[i]);

		strcpy(field->name, derefConstant(ci->constant_pool, fi->name_index));
		field->access_flags = fi->access_flags;
		parseType(derefConstant(ci->constant_pool, fi->descriptor_index), &(field->type));
	}

	return 0;
}

int parseCode(Method * m, MethodInfo * mi, ClassInfo * ci)
{
	char * attributeName;
	CodeInfo * codeInfo = NULL;

	for(int i = 0; i < mi->attributes_count; i++)
	{
		attributeName = derefConstant(ci->constant_pool, mi->attributes[i].attribute_name_index);
		if(strcmp(attributeName, "Code") == 0)
		{
			codeInfo = (CodeInfo *) mi->attributes[i].info;
			break;
		}
	}

	if(codeInfo == NULL)
	{
		printf("%s does not have a codeInfo attribute\n", m->name);
		exit(-1);
	}

	Instruction * currentInstrction = m->code;
	unsigned char * code = codeInfo->code;
	for(int i = 0; i < codeInfo->code_length; i++)
	{
		currentInstrction->op_code = code[i];
		int operand_count = getBytecodeOperandCount(code[i]);
		for(int j = 0; j < operand_count; j++)
		{
			i++;
			currentInstrction->operands[j].value = code[i]; 
		}
		currentInstrction++;
		m->instruction_count++;		
	}
}

int parseMethods(Class * c, ClassInfo * ci)
{
	c->method_count = ci->methods_count;

	for(int i = 0; i < c->method_count; i++)
	{
		Method * method = &( c->methods[i]);
		MethodInfo * mi = &(ci->methods[i]);

		strcpy(method->name, derefConstant(ci->constant_pool, mi->name_index));
		method->access_flags = mi->access_flags;
		
		char * descriptor = derefConstant(ci->constant_pool, mi->descriptor_index);
		
		// Skip the first open parenthesis
		descriptor++;
		while(*descriptor != ')')
		{
			parseType(descriptor, &(method->parameter_types[method->parameter_count]));
			
			Type * curType = &(method->parameter_types[method->parameter_count]);
			while(curType->type == TYPES_ARRAY)
			{
				curType->subtype = (Type *) malloc(sizeof(Type));
				curType = curType->subtype;
				descriptor++;
				parseType(descriptor, curType); 
			}

			// lets skip over reference types for now
			if(curType->type == TYPES_REFERENCE)
				while(*descriptor != ';') descriptor++;				

			method->parameter_count++;

			if(method->parameter_count > C_MAX_PARAMETERS)
			{
				printf("%s has more than he maximum number of parameters: %i\n", method->name, C_MAX_PARAMETERS);
				exit(-1);
			}

			descriptor++;
		}			
		descriptor++;
		parseType(descriptor, &(method->return_type));
		Type * curType = &(method->return_type);

		while(curType->type == TYPES_ARRAY)
		{
			curType->subtype = (Type *) malloc(sizeof(Type));
			curType = curType->subtype;
			descriptor++;
			parseType(descriptor, curType); 
		}

		parseCode(method, mi, ci);
	}

	return 0;
}

int parse(Class * c, ClassInfo * ci)
{
	c->constant_pool_count = ci->constant_pool_count;
	c->constant_pool = (CPInfo *) malloc(sizeof(CPInfo) * c->constant_pool_count);
	memcpy(c->constant_pool, ci->constant_pool, sizeof(CPInfo) * c->constant_pool_count);

	strcpy(c->name, derefConstant(ci->constant_pool, ci->constant_pool[ci->this_class].name_index));
	c->access_flags = ci->access_flags;
	c->super_class = NULL;

	parseFields(c, ci);
	parseMethods(c, ci);

	return 0;
}

char * printType(Type * t)
{
	char * returns = (char *) malloc(256);
	if(t->type == TYPES_ARRAY)
	{
		char * old = printType(t->subtype);
		sprintf(returns, "%s(%s)", TYPES_NAMES[t->type], old);
		free(old);
		return returns;
	}
	strcpy(returns, TYPES_NAMES[t->type]);
	return returns;	
}

int printFields(Class * c)
{
	printf("Field Count: %i\n", c->field_count);

	for(int i = 0; i < c->field_count; i++)
	{
		printf("\t%s %s: 0x%X\n", printType(&(c->fields[i].type)), c->fields[i].name, c->fields[i].access_flags);
	}
}

int printMethods(Class * c)
{
	printf("Method Count: %i\n", c->method_count);

	for(int i = 0; i < c->method_count; i++)
	{
		printf("\t%s %s: 0x%X\n", printType(&(c->methods[i].return_type)), c->methods[i].name, c->fields[i].access_flags);
		printf("\t\tParameter types: ");

		for(int j = 0; j < c->methods[i].parameter_count; j++)
		{
			printf("%s", printType(&(c->methods[i].parameter_types[j])));
			if( j != c->methods[i].parameter_count - 1 )
				printf(", ");
		}
		puts("");
		printf("\t\tNumber of instrctions: %i\n", c->methods[i].instruction_count);
		for(int j = 0; j < c->methods[i].instruction_count; j++)
		{
			printf("\t%2i: 0x%02x %s\n", j, c->methods[i].code[j].op_code, getBytecodeName(c->methods[i].code[j].op_code));
			for(int k = 0; k < getBytecodeOperandCount(c->methods[i].code[j].op_code); k++)
			{
				printf("\t\t0x%02x\n", c->methods[i].code[j].operands[k].value);
			}
		}
		puts("");
	}
}

int printClass(Class * c)
{
	printf("Class name: %s\n", c->name);
	printf("Access flags: %i\n", c->access_flags);
	printf("Number of Constants: %i\n", c->constant_pool_count);
	printConstantInfo(c->constant_pool, c->constant_pool_count);
	printFields(c);
	printMethods(c);
}
