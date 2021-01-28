#include "path_node.h"
#include <stdlib.h>

PathNode *newPathNode(char *path)
{
	PathNode *pathNode = malloc(sizeof (PathNode));
	pathNode->path = path;
	pathNode->next = NULL;
	return pathNode;
}
