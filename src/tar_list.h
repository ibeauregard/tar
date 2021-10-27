#ifndef TAR_LIST_H
#define TAR_LIST_H

#include "tar_node.h"
#include "params.h"

typedef struct {
	TarNode *const first;
	TarNode *last;
} TarList;

TarList getNewTarList(void);
int dumpToArchive(TarList *list, const Params *params, bool append);
int finalizeTarList(TarList *list);

#endif
