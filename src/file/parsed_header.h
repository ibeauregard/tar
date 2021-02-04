#ifndef HEADER_DATA_H
#define HEADER_DATA_H

#include "archive.h"
#include <stddef.h>
#include <sys/stat.h>

typedef struct stat Stat;

typedef struct s_HeaderData {
	char *path;
	char type;
	Stat *fileStat;
	size_t numBlocks;
} HeaderData;

int initHeaderData(HeaderData *headerData, char *path);
int finalizeHeaderData(HeaderData *headerData);

#endif
