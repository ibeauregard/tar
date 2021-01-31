#ifndef TAR_BUFFER_H
#define TAR_BUFFER_H

#include "archive.h"
#include <stddef.h>

typedef struct s_archived_file {
	const char *path;
	int fd;
	char *buffer;
	size_t numBlocks;
} ArchivedFile;

int initArchivedFile(ArchivedFile *file, const char *path, size_t numBlocks);
int readFile(ArchivedFile *file);
int writeToArchive(ArchivedFile *file, Archive *archive);
int closeArchivedFile(ArchivedFile *file);

#endif
