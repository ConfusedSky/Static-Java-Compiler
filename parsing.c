#include "scanning.h"

#ifdef PARSING_MAIN

int main(int argc, char const *argv[])
{
	ClassInfo ci;
	int returnValue = scanMain(argc, argv, &ci);

	if(returnValue != SCAN_NOT_OPENED)
	{
		CIFree(&ci);
	}

	return returnValue;
}

#endif
