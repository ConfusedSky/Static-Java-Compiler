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

int getVersionInfo(VersionInfo * info, FILE * file)
{
	ReturnError(readInt(&(info->minorVersion), 2, file));
	ReturnError(readInt(&(info->majorVersion), 2, file));

	return PARSE_OK;
}

int printConstantInfo(ConstantInfo * cf);

int getConstantInfo(ConstantInfo * cf, FILE * file)
{
	ReturnError(readInt(&(cf->nConstants), 2, file));
	cf->constantPool = (CPInfo *) malloc(sizeof(CPInfo) * cf->nConstants);

	for(int i = 1; i < cf->nConstants; i++)
	{
		CPInfo * info = &cf->constantPool[i];
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
				ReturnError(readInt((int *) &(info->length), 2, file));
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

int printConstantInfo(ConstantInfo * cf)
{
	for(int i = 1; i < cf->nConstants; i++)
	{
		CPInfo * info = &cf->constantPool[i];

		switch(info->tag)
		{
			case CONSTANT_Class:
				printf("%2i Class: Name index(%i)\n", i, info->name_index);
				break;
			case CONSTANT_Fieldref:
				printf("%2i Field Ref: Class index(%i), Name and type index(%i).\n", i, info->class_index, info->name_and_type_index);
				break;
			case CONSTANT_Methodref:
				printf("%2i Method Ref: Class index(%i), Name and type index(%i).\n", i, info->class_index, info->name_and_type_index);
				break;
			case CONSTANT_NameAndType:
				printf("%2i Name and Type info: Name index(%i), Descriptor index(%i)\n", i, info->name_index, info->descriptor_index);
				break;
			case CONSTANT_Utf8:
				printf("%2i UTF-8 string: %s\n", i, info->chars);
				break;
			default:
				printf("Unrecognized Constant Type: %i\n", info->tag);
		}
	}
}

int parse(FILE * file)
{
	VersionInfo vinfo;
	ConstantInfo cinfo;
	ReturnError(checkMagic(file));
	ReturnError(getVersionInfo(&vinfo, file));
	ReturnError(getConstantInfo(&cinfo, file));

	printConstantInfo(&cinfo);

	printf("Class Version: %i.%i\n", vinfo.majorVersion, vinfo.minorVersion);
	printf("Number of constants: %i\n", cinfo.nConstants);

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

	switch(parse(file))
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