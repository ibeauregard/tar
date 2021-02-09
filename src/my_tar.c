#include "argparsing/params.h"
#include "modes.h"
#include "error/error.h"
#include <stdlib.h>

int my_tar(int n_arguments, char **arguments)
{
	Params params = getNewParams();
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
			status = error("%s", PARSE_ERROR_MESSAGE);
	}
	if (status) {
		error("%s", PREVIOUS_ERROR_MESSAGE);
		return cleanupAfterFailure(&params);
	}
	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	return my_tar(argc - 1, argv + 1);
}
