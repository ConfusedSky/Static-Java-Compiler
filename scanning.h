#ifndef SCANNING_HEADER
#define SCANNING_HEADER

#define SCAN_OK 0
#define SCAN_NOT_OK 1
#define SCAN_FILE_CUT_OFF 2
#define SCAN_NOT_OPENED 3
#define SCAN_CONSTANT_TAG_NOT_DEFINED 4

int scan(ClassInfo * ci, FILE * file);

#endif
