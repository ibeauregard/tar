#ifndef TAR_LIST_H
#define TAR_LIST_H

#include "tar_node.h"
#include "argparsing/params.h"

typedef struct {
	TarNode *const first;
	TarNode *last;
} TarList;

TarList getNewTarList();
int dumpToArchive(TarList *list, const Params *params);
int finalizeTarList(TarList *list);

#endif
