#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "javastructures.h"
#include "scanning.h"

#define ReturnError(returnValue) {int r = returnValue; if(r){printf("Failed at line: %i\n", __LINE__); return r;}}
#define SWAP(a, b) {a ^= b; b ^= a; a ^= b;}

int reverseBytes(char * bytes, int nBytes)
{
	char * end = bytes + nBytes - 1;

	while(bytes < end)
	{
		SWAP(*bytes, *end);
		bytes++;
		end--;
	}

	return SCAN_OK;
}

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

		return SCAN_OK;
	}
	else
	{
		return SCAN_FILE_CUT_OFF;
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
		return SCAN_OK;
	}
	else
	{
		return SCAN_NOT_OK;
	}
}

int getVersionInfo(ClassInfo * info, FILE * file)
{
	ReturnError(readShort(&(info->minorVersion), file));
	ReturnError(readShort(&(info->majorVersion), file));

	return SCAN_OK;
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
			case CONSTANT_String:
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
					return SCAN_FILE_CUT_OFF;
				}
				info->chars[info->length] = 0;
				break;

			default:
				printConstantInfo(ci);
				return SCAN_CONSTANT_TAG_NOT_DEFINED;
		}
		
	}

	return SCAN_OK;
}

void printConstantInfo(ClassInfo * ci)
{
	for(int i = 1; i < ci->constant_pool_count; i++)
	{
		CPInfo * info = &ci->constant_pool[i];

		switch(info->tag)
		{
			case CONSTANT_String:
				printf("%2i String: Name index(%i)\n", i, info->string_index);
				printf("\tName: %s\n", derefConstant(ci, info->string_index));
				break;
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

	return SCAN_OK;
}

int get_source_file_info(AttributeInfo * ai)
{
	reverseBytes(ai->info, 2);
	return SCAN_OK;
}

int get_code_info(ClassInfo * ci, AttributeInfo * ai);

int getAttributeInfo(ClassInfo * ci, AttributeInfo * attribute)
{
	char * attribute_name = derefConstant(ci, attribute->attribute_name_index);
	if(strcmp(attribute_name, "SourceFile") == 0) get_source_file_info(attribute);
	if(strcmp(attribute_name, "Code"      ) == 0) get_code_info(ci, attribute);

	return SCAN_OK;
}
	
int getAttributes(ClassInfo * ci, AttributeInfo ** ai, short attributes_count, FILE * file)
{
	*ai = (AttributeInfo *) malloc(sizeof(AttributeInfo) * attributes_count);
	for(int i = 0; i < attributes_count; i++)
	{
		AttributeInfo * attribute = &((*ai)[i]);
		ReturnError(readShort(&attribute->attribute_name_index, file));
		ReturnError(readInt(&attribute->attribute_length, 4, file));

		attribute->info = (char *) malloc(attribute->attribute_length * sizeof(char));

		int bytesRead = fread(attribute->info, 1, attribute->attribute_length, file);
		if(bytesRead != attribute->attribute_length)
		{
			return SCAN_FILE_CUT_OFF;
		}
		
		return getAttributeInfo(ci, attribute);
	}

	return SCAN_OK;
}

int get_code_info(ClassInfo * ci, AttributeInfo * ai)
{
	char * info = ai->info;
	CodeInfo * code = (CodeInfo *) info;
	reverseBytes(info, 2);
	info += 2;
	reverseBytes(info, 2);
	info += 2;
	reverseBytes(info, 4);
	info += *(int *)info;
	info += 4;
	reverseBytes(info, 2);
	short exception_table_length = *(short *)info;
	ExceptionInfo * exception_table = (ExceptionInfo *) malloc(exception_table_length * sizeof(ExceptionInfo));
	info += 2;
	for(int i = 0; i < exception_table_length; i++)
	{
		reverseBytes(info, 2);
		exception_table[i].start_pc = *(short *) info;
		info += 2;
		reverseBytes(info, 2);
		exception_table[i].end_pc = *(short *) info;
		info += 2;
		reverseBytes(info, 2);
		exception_table[i].handler_pc = *(short *) info;
		info += 2;
		reverseBytes(info, 2);
		exception_table[i].catch_type = *(short *) info;
		info += 2;
	}

	reverseBytes(info, 2);
	short attributes_count = *(short *) info;
	info += 2;

	AttributeInfo * attributes = (AttributeInfo *) malloc(attributes_count * sizeof(AttributeInfo));

	for(int i = 0; i < attributes_count; i++)
	{
		reverseBytes(info, 2);
		attributes[i].attribute_name_index = *(short *) info;
		info += 2;
		reverseBytes(info, 4);
		attributes[i].attribute_length = *(int *) info;
		info += 4;

		attributes[i].info = malloc(attributes[i].attribute_length);
		memcpy((void *) &(attributes[i].info), info, attributes[i].attribute_length);
		info += attributes[i].attribute_length;

		getAttributeInfo(ci, &(attributes[i]));
	}

	code->exception_table_length = exception_table_length;
	code->exception_table = exception_table;
	code->attributes_count = attributes_count;
	code->attributes = attributes;

	return SCAN_OK;
}

int print_ExceptionInfo(ExceptionInfo * ei)
{
	printf("\t\t\tStart_PC: %i\n", ei->start_pc);
	printf("\t\t\tEnd_PC: %i\n", ei->end_pc);
	printf("\t\t\tHandler_PC: %i\n", ei->handler_pc);
	printf("\t\t\tCatch_Type: %i\n", ei->catch_type);
}

int printAttributesD(ClassInfo *, AttributeInfo *, short, int depth);

int print_code_info(ClassInfo * ci, char * info)
{
	CodeInfo * code = (CodeInfo *) info;
	printf("\t\tMax Stack: %i\n", code->max_stack);
	printf("\t\tMax Locals: %i\n", code->max_locals);
	printf("\t\tCode Length: %i\n", code->code_length);
	printf("\t\tException Table Length: %i\n", code->exception_table_length);
	for(int i = 0; i < code->exception_table_length; i++)
	{
		print_ExceptionInfo(&code->exception_table[i]);
		puts("");
	}
	printf("\t\tAttributes Count: %i\n", code->attributes_count);
	printAttributesD(ci, code->attributes, code->attributes_count, 2);
}

int putTabs(int depth)
{
	for(int i = 0; i < depth; i++)
	{
		putchar('\t');
	}
}

int printAttributesD(ClassInfo * ci, AttributeInfo * ai, short attributes_count, int depth)
{
	for(int i = 0; i < attributes_count; i++)
	{
		char * attribute_name = derefConstant(ci, ai[i].attribute_name_index);
		putTabs(depth);
		printf("\t%s: ", attribute_name);
		if(strcmp(attribute_name, "SourceFile") == 0)
		{
			SourceFileInfo * source_file = (SourceFileInfo *) ai[i].info;
			unsigned index = source_file->sourcefile_index;
			printf("%s", derefConstant(ci, index));
		}
		if(strcmp(attribute_name, "Code") == 0 )
		{
			puts("");
			print_code_info(ci, ai[i].info);
		}
		puts("");
		putTabs(depth);
		printf("\t\tLength: %i\n", ai[i].attribute_length);
	}
}

int printAttributes(ClassInfo * ci, AttributeInfo * ai, short attributes_count)
{
	printAttributesD(ci, ai, attributes_count, 0);
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
		ReturnError(getAttributes(ci, &field->attributes, field->attributes_count, file));
	}

	return SCAN_OK;
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
		ReturnError(getAttributes(ci, &method->attributes, method->attributes_count, file));
	}

	return SCAN_OK;
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
	ReturnError(getAttributes(ci, &ci->attributes, ci->attributes_count, file));

	return SCAN_OK;
}

void printClassInfo(ClassInfo * ci)
{
	printf("Class Version: %i.%i\n", ci->majorVersion, ci->minorVersion);
	printf("Number of Constants: %i\n", ci->constant_pool_count);
	printConstantInfo(ci);
	printf("Access Flags: %#06x\n", ci->access_flags);
	printf("Class: %s\n", derefConstant(ci, ci->constant_pool[ci->this_class].name_index));
	printf("Super Class: %s\n", derefConstant(ci, ci->constant_pool[ci->super_class].name_index));
	printf("Number of Interfaces: %i\n", ci->interfaces_count);
	printf("Number of Fields: %i\n", ci->fields_count);
	printf("\nNumber of Methods: %i\n", ci->methods_count);
	printMethods(ci);
	printf("\nNumber of Attributes: %i\n", ci->attributes_count);
	printAttributes(ci, ci->attributes, ci->attributes_count);
}

#ifdef SCANNING_MAIN 
int main(int argc, char const *argv[])
{
	if(argc < 2)
	{
		puts("Format: ./scanning filename\n");
		return 1;
	}

	FILE * file;
	file = fopen(argv[1], "rb");
	if(!file)
	{
		puts("File was not opened.\n");
		return SCAN_NOT_OPENED;
	}

	ClassInfo ci;
	int returnValue = scan(&ci, file);

	switch(returnValue)
	{
		case SCAN_OK:
			puts("File correct format.\n");
			break;
		case SCAN_NOT_OK:
			puts("File incorrect format.\n");
			break;
		case SCAN_FILE_CUT_OFF:
			puts("File was cut off.\n");
			break;
		case SCAN_CONSTANT_TAG_NOT_DEFINED:
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
#endif
