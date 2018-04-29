#include "scanning.h"

#ifdef PARSING_MAIN

int main(int argc, char const *argv[])
{
	ClassInfo ci;
	int returnValue = scanMain(argc, argv, &ci);

	if(returnValue == SCAN_OK)
	{
		CIFree(&ci);
	}

	return returnValue;
}

#endif
