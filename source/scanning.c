#include <stdio.h>
#include <stdlib.h>
#include "javastructures.h"

#define PARSE_OK 0
#define PARSE_NOT_OK 1
#define PARSE_FILE_CUT_OFF 2
#define PARSE_NOT_OPENED 3
#define PARSE_CONSTANT_TAG_NOT_DEFINED 4

#define ReturnError(returnValue) {int r = returnValue; if(r) return r;}

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

int printConstantInfo(ClassInfo * cf);

int getConstantInfo(ClassInfo * cf, FILE * file)
{
	ReturnError(readShort(&(cf->constant_pool_count), file));
	cf->constant_pool = (CPInfo *) malloc(sizeof(CPInfo) * cf->constant_pool_count);

	for(int i = 1; i < cf->constant_pool_count; i++)
	{
		CPInfo * info = &cf->constant_pool[i];
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
				printConstantInfo(cf);
				return PARSE_CONSTANT_TAG_NOT_DEFINED;
		}
		
	}

	return PARSE_OK;
}

int printConstantInfo(ClassInfo * cf)
{
	for(int i = 1; i < cf->constant_pool_count; i++)
	{
		CPInfo * info = &cf->constant_pool[i];

		switch(info->tag)
		{
			case CONSTANT_Class:
				printf("%2i Class: Name index(%i)\n", i, info->name_index);
				printf("\tName: %s\n",cf->constant_pool[info->name_index].chars);
				break;
			case CONSTANT_Fieldref:
				printf("%2i Field Ref: Class index(%i), Name and type index(%i).\n", i, info->class_index, info->name_and_type_index);
				printf("\tClass: %s\n", cf->constant_pool[cf->constant_pool[info->class_index].name_index].chars);
				printf("\tName: %s\n", cf->constant_pool[cf->constant_pool[info->name_and_type_index].name_index].chars);
				printf("\tDescriptor: %s\n", cf->constant_pool[cf->constant_pool[info->name_and_type_index].descriptor_index].chars);
				break;
			case CONSTANT_Methodref:
				printf("%2i Method Ref: Class index(%i), Name and type index(%i).\n", i, info->class_index, info->name_and_type_index);
				printf("\tClass: %s\n", cf->constant_pool[cf->constant_pool[info->class_index].name_index].chars);
				printf("\tName: %s\n", cf->constant_pool[cf->constant_pool[info->name_and_type_index].name_index].chars);
				printf("\tDescriptor: %s\n", cf->constant_pool[cf->constant_pool[info->name_and_type_index].descriptor_index].chars);
				break;
			case CONSTANT_NameAndType:
				printf("%2i Name and Type info: Name index(%i), Descriptor index(%i)\n", i, info->name_index, info->descriptor_index);
				printf("\tName: %s\n", cf->constant_pool[info->name_index].chars);
				printf("\tDescriptor: %s\n", cf->constant_pool[info->descriptor_index].chars);
				break;
			case CONSTANT_Utf8:
				printf("%2i UTF-8 string: %s\n", i, info->chars);
				break;
			default:
				printf("Unrecognized Constant Type: %i\n", info->tag);
		}
	}
}

int scan(FILE * file)
{
	ClassInfo class;
	ReturnError(checkMagic(file));
	ReturnError(getVersionInfo(&class, file));
	ReturnError(getConstantInfo(&class, file));

	printf("Class Version: %i.%i\n", class.majorVersion, class.minorVersion);
	printf("Number of constants: %i\n", class.constant_pool_count);

	return PARSE_OK;
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

	int returnValue = PARSE_OK;

	switch(scan(file))
	{
		case PARSE_OK:
			puts("File correct format.\n");
			break;
		case PARSE_NOT_OK:
			puts("File incorrect format.\n");
			returnValue = PARSE_NOT_OK;
			break;
		case PARSE_FILE_CUT_OFF:
			puts("File was cut off.\n");
			returnValue = PARSE_FILE_CUT_OFF;
			break;
		case PARSE_CONSTANT_TAG_NOT_DEFINED:
			puts("Constant tag not defined.\n");
			returnValue = PARSE_CONSTANT_TAG_NOT_DEFINED;
			break;
	}

	fclose(file);
	return PARSE_OK;
}