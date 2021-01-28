#ifndef _STRING_H
#define _STRING_H

#include <stdbool.h>
#include <stddef.h>

typedef unsigned int uint;

bool starts_with(const char* str, char c);
size_t _strlen(const char* str);
#endif
