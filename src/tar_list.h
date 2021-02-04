#ifndef TAR_LIST_H
#define TAR_LIST_H

#include "tar_node.h"

typedef struct {
	TarNode *node;
	TarNode *last;
} TarList;

TarList getNewTarList();
int dumpToArchive(TarList *list, const char *archivePath);
int finalizeTarList(TarList *list);

#endif
