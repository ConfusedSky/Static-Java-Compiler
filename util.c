#include "util.h"
#include "scanning.h"

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
