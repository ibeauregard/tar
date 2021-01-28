#include "_string.h"

bool starts_with(const char* str, char c)
{
	return str && str[0] == c;
}

size_t _strlen(const char* str)
{
	size_t len;
	for (len = 0; str[len]; len++);
	return len;
}
