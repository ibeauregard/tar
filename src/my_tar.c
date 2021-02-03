#include "utils/_stdio.h"
#include "argparsing/params.h"
#include <stdlib.h>
#include <unistd.h>
#include "modes.h"
#include "error/error.h"

int my_tar(int n_arguments, char **arguments)
{
	Params params;
	if (parseArguments(n_arguments, arguments, &params)) {
		return cleanupAfterFailure(&params);
	}
	int status;
	switch (params.mode) {
		case C:
			status = c_mode(&params);
			break;
		case R:
			status = r_mode(&params);
			break;
		case T:
			status = t_mode(&params);
			break;
		case U:
			status = u_mode(&params);
			break;
		case X:
			status = x_mode(&params);
			break;
		default:
			status = EXIT_FAILURE;
			_dprintf(STDERR_FILENO, "%s\n", PARSE_ERROR_MESSAGE);
	}
	if (status) {
		_dprintf(STDERR_FILENO, "%s", PREVIOUS_ERROR_MESSAGE);
		return cleanupAfterFailure(&params);
	}
	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	return my_tar(argc - 1, argv + 1);
}
