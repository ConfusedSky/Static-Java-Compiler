#include "javastructures.h"
#include "cstructures.h"
#include "scanning.h"
#include "parsing.h"
#include "interpretting.h"
#include "string.h"

int DEBUG = 0;

int main(int argc, const char *argv[])
{
	ClassInfo classinfo;
	ClassInfo * ci = &classinfo;

	Class class;
	Class * c = &class;

	if(argc < 2)
	{
		printf("Format: %s filename\n\n", argv[0]);
		return SCAN_NOT_OPENED;
	}

	if(argc > 2)
	{
		if(strcmp(argv[2], "debug") == 0)
		{
			DEBUG = 1;
		}
	}

	FILE * file;
	file = fopen(argv[1], "rb");
	if(!file)
	{
		puts("File was not opened.\n");
		return SCAN_NOT_OPENED;
	}

	int returnValue = scan(ci, file);

	if(DEBUG) switch(returnValue)
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
#if defined(PARSING_MAIN) || defined(INTERPRETTING_MAIN)
		returnValue = parse(c, ci);
#endif
#ifdef PARSING_MAIN
		printClass(c);
#endif
#ifdef INTERPRETTING_MAIN
		interpret(c);
#endif
		CIFree(ci);
	}


	fclose(file);
	return returnValue;
}

