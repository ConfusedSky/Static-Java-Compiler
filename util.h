#ifndef UTILS_HEADER
#define UTILS_HEADER

#define SWAP(a, b) {a ^= b; b ^= a; a ^= b;}
#define ReturnError(returnValue) {int r = returnValue; if(r){printf("Failed at line: %i\n", __LINE__); return r;}}

int reverseBytes(char * bytes, int nbytes);

#endif
