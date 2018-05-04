#ifndef SCANNING_HEADER
#define SCANNING_HEADER

#include "javastructures.h"
#include <stdio.h>

#define SCAN_OK 0
#define SCAN_NOT_OK 1
#define SCAN_FILE_CUT_OFF 2
#define SCAN_NOT_OPENED 3
#define SCAN_CONSTANT_TAG_NOT_DEFINED 4

int scan(ClassInfo * ci, FILE * file);

// CIFree must be called after the ClassInfo is done being used
// If CIFree is not called there will be significant memory leaks
void CIFree(ClassInfo * ci);

void printClassInfo(ClassInfo * ci);

#endif
