#include "tar_node.h"
#include <stdlib.h>

TarNode *getNewTarNode(HeaderData *headerData)
{
	TarNode *node = malloc(sizeof (TarNode));
	node->headerData = headerData;
	node->contents = 0;
	node->next = 0;
	return node;
}
