#ifndef TAR_BUFFER_H
#define TAR_BUFFER_H

#include "archive_file.h"
#include <stddef.h>

typedef struct s_archived_file {
	const char *path;
	char *buffer;
	size_t numBlocks;
} ArchivedFile;

ArchivedFile *newArchivedFile(const char *path, size_t numBlocks);
int readFromFd(ArchivedFile *file, int fd);
int writeToArchive(ArchivedFile *file, ArchiveFile *archive);
void freeArchivedFile(ArchivedFile *file);

#endif
