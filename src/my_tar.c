#include "_stdio.h"
#include "params.h"
#include <stdlib.h>
#include <unistd.h>
#include "modes.h"

#define PARSE_ERROR_MESSAGE "Internal application error: Failed to parse mode in params.c\n"

int my_tar(int n_arguments, char **arguments)
{
	Params params;
	if (parseArguments(n_arguments, arguments, &params)) {
		return EXIT_FAILURE;
	}
	switch (params.mode) {
		case C:
			return c_mode(&params);
		case R:
			return r_mode(&params);
		case T:
			return t_mode(&params);
		case U:
			return u_mode(&params);
		case X:
			return x_mode(&params);
		default:
			_dprintf(STDERR_FILENO, "%s\n", PARSE_ERROR_MESSAGE);
	}
	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	return my_tar(argc - 1, argv + 1);
}
