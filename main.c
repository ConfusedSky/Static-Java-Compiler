#include "javastructures.h"
#include "scanning.h"
#include "parsing.h"

int main(int argc, char const *argv[])
{
	ClassInfo class;
	ClassInfo * ci = &class;

	if(argc < 2)
	{
		printf("Format: %s filename\n\n", argv[0]);
		return SCAN_NOT_OPENED;
	}

	FILE * file;
	file = fopen(argv[1], "rb");
	if(!file)
	{
		puts("File was not opened.\n");
		return SCAN_NOT_OPENED;
	}

	int returnValue = scan(ci, file);

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

	if(returnValue == SCAN_OK)
	{
#ifdef SCANNING_MAIN
		printClassInfo(ci);
#endif
		CIFree(ci);
	}


	fclose(file);
	return returnValue;
}

