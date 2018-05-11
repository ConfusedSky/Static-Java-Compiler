#include "parsing.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>

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

		strcpy(field->name, derefConstant(ci, fi->name_index));
		field->access_flags = fi->access_flags;
		parseType(derefConstant(ci, fi->descriptor_index), &(field->type));
	}

	return 0;
}

int parseMethods(Class * c, ClassInfo * ci)
{
	c->method_count = ci->methods_count;

	for(int i = 0; i < c->method_count; i++)
	{
		Method * method = &( c->methods[i]);
		MethodInfo * mi = &(ci->methods[i]);

		strcpy(method->name, derefConstant(ci, mi->name_index));
		method->access_flags = mi->access_flags;
		
		char * descriptor = derefConstant(ci, mi->descriptor_index);
		
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
	}

	return 0;
}

int parse(Class * c, ClassInfo * ci)
{
	strcpy(c->name, derefConstant(ci, ci->constant_pool[ci->this_class].name_index));
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
	}
}

int printClass(Class * c)
{
	printf("Class name: %s\n", c->name);
	printf("Access flags: %i\n", c->access_flags);
	printFields(c);
	printMethods(c);
}
