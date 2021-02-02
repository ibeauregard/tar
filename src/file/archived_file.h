#ifndef TAR_BUFFER_H
#define TAR_BUFFER_H

#include "archive.h"
#include <stddef.h>
#include <sys/stat.h>

typedef struct stat Stat;

typedef struct s_archived_file {
	char *path;
	char type;
	int fd;
	Stat *fileStat;
	char *buffer;
	size_t numBlocks;
} ArchivedFile;

int initArchivedFile(ArchivedFile *file, char *path);
int readFile(const ArchivedFile *file);
int writeToArchive(const ArchivedFile *file, Archive *archive);
int destructArchivedFile(const ArchivedFile *file);

#endif
