#include "_stdio.h"
#include "params.h"
#include <stdlib.h>

int my_tar(int n_arguments, char **arguments)
{
	Params params;
	int returnValue = parse_arguments(n_arguments, arguments, &params);
	_puts("Result of arg parsing:");
	_printf("Mode: %d\n", params.mode);
	_printf("Archive path: %s\n", params.archivePath);
	_puts("File arguments:");
	PathNode *pathNode = params.filePaths;
	while (pathNode) {
		_printf("Path: %s\n", pathNode->path);
		PathNode *current = pathNode;
		pathNode = pathNode->next;
		free(current);
	}
	return returnValue;
}

int main(int argc, char **argv)
{
	return my_tar(argc - 1, argv + 1);
}
