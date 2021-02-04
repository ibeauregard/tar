#include "tar_node.h"
#include <stdlib.h>

TarNode *getNewTarNode(ParsedHeader *parsedHeader)
{
	TarNode *node = malloc(sizeof (TarNode));
	node->parsedHeader = parsedHeader;
	node->contents = 0;
	node->next = 0;
	return node;
}
