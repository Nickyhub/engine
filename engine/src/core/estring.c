#include <string.h>
#include "estring.h"

b8 string_compare(const char* str0, const char* str1) {
	return strcmp(str0, str1) == 0;
}