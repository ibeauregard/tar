#ifndef TAR_NODE_H
#define TAR_NODE_H

#include "tar_header.h"

typedef struct s_TarNode {
	HeaderData *headerData;
	PosixHeader *header;
	char *contents;
	struct s_TarNode *next;
} TarNode;

TarNode *getNewTarNode(HeaderData *headerData);

#endif
