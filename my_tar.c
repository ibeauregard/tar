#include "_stdio.h"

int my_tar(int n_arguments, char **arguments)
{
	_printf("Called my_tar with %d arguments:\n", n_arguments);
	for (int i = 0; i < n_arguments; i++) {
		_puts(arguments[i]);
	}
	return 0;
}

int main(int argc, char **argv)
{
	return my_tar(argc - 1, argv + 1);
}
