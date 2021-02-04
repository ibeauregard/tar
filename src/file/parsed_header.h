#ifndef ARCHIVED_FILE_H
#define ARCHIVED_FILE_H

#include "archive.h"
#include <stddef.h>
#include <sys/stat.h>

typedef struct stat Stat;

typedef struct s_ParsedHeader {
	char *path;
	char type;
	Stat *fileStat;
	size_t numBlocks;
} ParsedHeader;

int initParsedHeader(ParsedHeader *parsedHeader, char *path);
int finalizeParsedHeader(ParsedHeader *parsedHeader);

#endif
