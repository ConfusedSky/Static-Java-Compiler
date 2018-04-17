#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "javastructures.h"

#define PARSE_OK 0
#define PARSE_NOT_OK 1
#define PARSE_FILE_CUT_OFF 2
#define PARSE_NOT_OPENED 3
#define PARSE_CONSTANT_TAG_NOT_DEFINED 4

#define ReturnError(returnValue) {int r = returnValue; if(r){printf("Failed at line: %i\n", __LINE__); return r;}}

int readInt(int * value, int nBytes, FILE * file)
{
	unsigned char bytes[4];
	*value = 0;
	unsigned int bytesRead = fread(&bytes, 1, nBytes, file);

	if(bytesRead == nBytes)
	{
		for(int i = 0; i < nBytes; i++)
		{
			*value += bytes[i] << (8 * (nBytes - 1 - i));
		}

		return PARSE_OK;
	}
	else
	{
		return PARSE_FILE_CUT_OFF;
	}
}

int readShort(short * value, FILE * file)
{
	return readInt((int *) value, 2, file);
}

char * derefConstant(ClassInfo * ci, int name_index)
{
	return ci->constant_pool[name_index].chars;
}

int checkMagic(FILE * file)
{
	unsigned int magic;

	ReturnError(readInt(&magic, 4, file));

	if(!(magic^0xCAFEBABE))
	{
		return PARSE_OK;
	}
	else
	{
		return PARSE_NOT_OK;
	}
}

int getVersionInfo(ClassInfo * info, FILE * file)
{
	ReturnError(readShort(&(info->minorVersion), file));
	ReturnError(readShort(&(info->majorVersion), file));

	return PARSE_OK;
}

void printConstantInfo(ClassInfo * ci);

int getConstantInfo(ClassInfo * ci, FILE * file)
{
	ReturnError(readShort(&(ci->constant_pool_count), file));
	ci->constant_pool = (CPInfo *) malloc(sizeof(CPInfo) * ci->constant_pool_count);

	for(int i = 1; i < ci->constant_pool_count; i++)
	{
		CPInfo * info = &ci->constant_pool[i];
		ReturnError(readInt((int *) &(info->tag), 1, file));

		switch(info->tag)
		{
			case CONSTANT_Class:
				ReturnError(readInt(&(info->name_index), 2, file));
				break;

			case CONSTANT_Fieldref:
			case CONSTANT_Methodref:
				ReturnError(readInt(&(info->class_index), 2, file));
				ReturnError(readInt(&(info->name_and_type_index), 2, file));
				break;

			case CONSTANT_NameAndType:
				ReturnError(readInt(&(info->name_index), 2, file));
				ReturnError(readInt(&(info->descriptor_index), 2, file));
				break;

			case CONSTANT_Utf8:
				ReturnError(readShort(&(info->length), file));
				info->chars = (char *) malloc(sizeof(char) * (info->length+1));
				int bytesRead = fread(info->chars, 1, info->length, file);
				if(bytesRead != info->length)
				{
					return PARSE_FILE_CUT_OFF;
				}
				info->chars[info->length] = 0;
				break;

			default:
				printConstantInfo(ci);
				return PARSE_CONSTANT_TAG_NOT_DEFINED;
		}
		
	}

	return PARSE_OK;
}

void printConstantInfo(ClassInfo * ci)
{
	for(int i = 1; i < ci->constant_pool_count; i++)
	{
		CPInfo * info = &ci->constant_pool[i];

		switch(info->tag)
		{
			case CONSTANT_Class:
				printf("%2i Class: Name index(%i)\n", i, info->name_index);
				printf("\tName: %s\n", derefConstant(ci, info->name_index));
				break;
			case CONSTANT_Fieldref:
				printf("%2i Field Ref: Class index(%i), Name and type index(%i).\n", i, info->class_index, info->name_and_type_index);
				printf("\tClass: %s\n", derefConstant(ci, ci->constant_pool[info->class_index].name_index));
				printf("\tName: %s\n", derefConstant(ci, ci->constant_pool[info->name_and_type_index].name_index));
				printf("\tDescriptor: %s\n", derefConstant(ci, ci->constant_pool[info->name_and_type_index].descriptor_index));
				break;
			case CONSTANT_Methodref:
				printf("%2i Method Ref: Class index(%i), Name and type index(%i).\n", i, info->class_index, info->name_and_type_index);
				printf("\tClass: %s\n", derefConstant(ci, ci->constant_pool[info->class_index].name_index));
				printf("\tName: %s\n", derefConstant(ci, ci->constant_pool[info->name_and_type_index].name_index));
				printf("\tDescriptor: %s\n", derefConstant(ci, ci->constant_pool[info->name_and_type_index].descriptor_index));
				break;
			case CONSTANT_NameAndType:
				printf("%2i Name and Type info: Name index(%i), Descriptor index(%i)\n", i, info->name_index, info->descriptor_index);
				printf("\tName: %s\n", derefConstant(ci, info->name_index));
				printf("\tDescriptor: %s\n", derefConstant(ci, info->descriptor_index));
				break;
			case CONSTANT_Utf8:
				printf("%2i UTF-8 string: %s\n", i, info->chars);
				break;
			default:
				printf("Unrecognized Constant Type: %i\n", info->tag);
		}
	}
}

int getInterfaces(ClassInfo * ci, FILE * file)
{
	ReturnError(readShort(&ci->interfaces_count, file));
	ci->interfaces = (short *) malloc(sizeof(short) * ci->interfaces_count);
	for(int i = 0; i < ci->interfaces_count; i++)
	{
		ReturnError(readShort(&ci->interfaces[i], file)); 
	}

	return PARSE_OK;
}

int getAttributes(AttributeInfo ** ai, short attributes_count, FILE * file)
{
	*ai = (AttributeInfo *) malloc(sizeof(AttributeInfo) * attributes_count);
	for(int i = 0; i < attributes_count; i++)
	{
		AttributeInfo * attribute = &*ai[i];
		ReturnError(readShort(&attribute->attribute_name_index, file));
		ReturnError(readInt(&attribute->attribute_length, 4, file));
		attribute->info = (char *) malloc(attribute->attribute_length * sizeof(chars));

		int bytesRead = fread(attribute->info, 1, attribute->attribute_length, file);
		if(bytesRead != attribute->attribute_length)
		{
			return PARSE_FILE_CUT_OFF;
		}
	}

	return PARSE_OK;
}

int printAttributes(ClassInfo * ci, AttributeInfo * ai, short attributes_count)
{
	for(int i = 0; i < attributes_count; i++)
	{
		char * attribute_name = derefConstant(ci, ai[i].attribute_name_index);
		printf("\t%s: ", attribute_name);
		if(strcmp(attribute_name, "SourceFile") == 0)
		{
			printf("%s\n", derefConstant(ci, *(short *) ai[i].info));
		}
		puts("");
		printf("\t\tLength: %i\n", ai[i].attribute_length);
	}
}

int getFields(ClassInfo * ci, FILE * file)
{
	ReturnError(readShort(&ci->fields_count, file));
	ci->fields = (FieldInfo *) malloc(sizeof(FieldInfo) * ci->fields_count);
	for(int i = 0; i < ci->fields_count; i++)
	{
		FieldInfo * field = &ci->fields[i];
		ReturnError(readShort(&field->access_flags, file));
		ReturnError(readShort(&field->name_index, file));
		ReturnError(readShort(&field->descriptor_index, file));
		ReturnError(readShort(&field->attributes_count, file));
		ReturnError(getAttributes(&field->attributes, field->attributes_count, file));
	}

	return PARSE_OK;
}

void printFields(ClassInfo * ci)
{
	for(int i = 0; i < ci->fields_count; i++)
	{
		printf("%#06x,%s,%s\n", ci->fields[i].access_flags, derefConstant(ci, ci->fields[i].name_index), derefConstant(ci, ci->fields[i].descriptor_index));
	}
}

int getMethods(ClassInfo * ci, FILE * file)
{
	ReturnError(readShort(&ci->methods_count, file));
	ci->methods = (MethodInfo *) malloc(sizeof(MethodInfo) * ci->methods_count);
	for(int i = 0; i < ci->methods_count; i++)
	{
		MethodInfo * method = &ci->methods[i];
		ReturnError(readShort(&method->access_flags, file));
		ReturnError(readShort(&method->name_index, file));
		ReturnError(readShort(&method->descriptor_index, file));
		ReturnError(readShort(&method->attributes_count, file));
		ReturnError(getAttributes(&method->attributes, method->attributes_count, file));
	}

	return PARSE_OK;
}

void printMethods(ClassInfo * ci)
{
	for(int i = 0; i < ci->methods_count; i++)
	{
		MethodInfo * method = &ci->methods[i];
		printf("%#06x, %s, %s\n", method->access_flags, derefConstant(ci, method->name_index), derefConstant(ci, method->descriptor_index));
		printAttributes(ci, method->attributes, method->attributes_count);
	}
}

int scan(ClassInfo * ci, FILE * file)
{
	ReturnError(checkMagic(file));
	ReturnError(getVersionInfo(ci, file));
	ReturnError(getConstantInfo(ci, file));
	ReturnError(readShort(&ci->access_flags, file));
	ReturnError(readShort(&ci->this_class, file));
	ReturnError(readShort(&ci->super_class, file));
	ReturnError(getInterfaces(ci, file));
	ReturnError(getFields(ci, file));
	ReturnError(getMethods(ci, file));
	ReturnError(readShort(&ci->attributes_count, file));
	ReturnError(getAttributes(&ci->attributes, ci->attributes_count, file));

	return PARSE_OK;
}

void printClassInfo(ClassInfo * ci)
{
	printf("Class Version: %i.%i\n", ci->majorVersion, ci->minorVersion);
	printf("Number of Constants: %i\n", ci->constant_pool_count);
	printConstantInfo(ci);
	printf("\nAccess Flags: %#06x\n", ci->access_flags);
	printf("Class: %s\n", derefConstant(ci, ci->constant_pool[ci->this_class].name_index));
	printf("Super Class: %s\n", derefConstant(ci, ci->constant_pool[ci->super_class].name_index));
	printf("Number of Interfaces: %i\n", ci->interfaces_count);
	printf("Number of Fields: %i\n", ci->fields_count);
	printf("\nNumber of Methods: %i\n", ci->methods_count);
	printMethods(ci);
	printf("\nNumber of Attributes: %i\n", ci->attributes_count);
	printAttributes(ci, ci->attributes, ci->attributes_count);
}

int main(int argc, char const *argv[])
{
	if(argc < 2)
	{
		puts("Format: ./parsing filename\n");
		return 1;
	}

	FILE * file;
	file = fopen(argv[1], "rb");
	if(!file)
	{
		puts("File was not opened.\n");
		return PARSE_NOT_OPENED;
	}

	ClassInfo ci;
	int returnValue = scan(&ci, file);

	switch(returnValue)
	{
		case PARSE_OK:
			puts("File correct format.\n");
			break;
		case PARSE_NOT_OK:
			puts("File incorrect format.\n");
			break;
		case PARSE_FILE_CUT_OFF:
			puts("File was cut off.\n");
			break;
		case PARSE_CONSTANT_TAG_NOT_DEFINED:
			puts("Constant tag not defined.\n");
			break;
		default:
			printf("Unknown return code: %i\n", returnValue);
			break;
	}

	printClassInfo(&ci);

	fclose(file);
	return returnValue;
}
