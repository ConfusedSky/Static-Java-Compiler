#include <stdio.h>

#define PARSE_OK 0
#define PARSE_NOT_OK 1
#define PARSE_FILE_CUT_OFF 2
#define PARSE_NOT_OPENED 3;

#define ReturnError(returnValue) if(returnValue) return returnValue

unsigned int readInt(int * value, int nBytes, FILE * file)
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
	int returnValue = readInt(&magic, 4, file);

	ReturnError(returnValue);

	if(!(magic^0xCAFEBABE))
	{
		return PARSE_OK;
	}
	else
	{
		return PARSE_NOT_OK;
	}
}

typedef struct _versionInfo
{
	int majorVersion;
	int minorVersion;
} VersionInfo;

int getVersionInfo(VersionInfo * info, FILE * file)
{
	int returnValue = readInt(&(info->minorVersion), 2, file);
	ReturnError(returnValue);
	returnValue = readInt(&(info->majorVersion), 2, file);
	ReturnError(returnValue);

	return PARSE_OK;
}

int parse(FILE * file)
{
	VersionInfo info;
	int returnValue = checkMagic(file);
	ReturnError(returnValue);
	returnValue = getVersionInfo(&info, file);
	ReturnError(returnValue);

	printf("Class Version: %i.%i\n", info.majorVersion, info.minorVersion);

	return returnValue;
}

int main(int argc, char const *argv[])
{
	if(argc < 2)
	{
		puts("Format: ./parsing filename");
		return 1;
	}

	FILE * file;
	file = fopen(argv[1], "rb");
	if(!file)
	{
		puts("File was not opened");
		return PARSE_NOT_OPENED;
	}

	switch(parse(file))
	{
		case PARSE_OK:
			puts("File correct format");
			break;
		case PARSE_NOT_OK:
			puts("File incorrect format");
			return PARSE_NOT_OK;
		case PARSE_FILE_CUT_OFF:
			puts("File was cut off");
			return PARSE_FILE_CUT_OFF;
	}

	return PARSE_OK;
}