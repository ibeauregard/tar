#include "modes.h"
#include <stdlib.h>

int u_mode(Params *params)
{
	// THIS IS JUST TO PREVENT ANY MEMORY LEAK PENDING IMPLEMENTATION
	PathNode *pathNode = params->filePaths;
	while (pathNode) {
		PathNode *current = pathNode;
		pathNode = pathNode->next;
		free(current);
	}
	return EXIT_SUCCESS;
}
