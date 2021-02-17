#include "error.h"
#include "utils/_stdio.h"
#include "stdlib.h"
#include <unistd.h>

int error(const char *message, const char *messageArg)
{
	_dprintf(STDERR_FILENO, message, messageArg);
	return EXIT_FAILURE;
}

int errorCharArg(const char *message, char messageArg)
{
	_dprintf(STDERR_FILENO, message, messageArg);
	return EXIT_FAILURE;
}
