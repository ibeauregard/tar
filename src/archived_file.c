#include "archived_file.h"
#include "tar_header.h"
#include "unistd.h"
#include <stdlib.h>

static void zfill(ArchivedFile *buffer);

ArchivedFile *newArchivedFile(const char *path, size_t numBlocks)
{
	ArchivedFile *file = malloc(sizeof (ArchivedFile));
	file->path = path;
	file->buffer = malloc((numBlocks + 1) * BLOCKSIZE);
	file->numBlocks = numBlocks;
	zfill(file);
	return file;
}

int readFromFd(ArchivedFile *file, int fd)
{
	return read(fd, file->buffer + BLOCKSIZE, file->numBlocks * BLOCKSIZE);
}

int writeToArchive(ArchivedFile *file, ArchiveFile *archive)
{
	return write(archive->fd, file->buffer, (file->numBlocks + 1) * BLOCKSIZE);
}

void freeArchivedFile(ArchivedFile *file)
{
	free(file->buffer);
	free(file);
}

void zfill(ArchivedFile *file)
{
	size_t i;
	for (i = 0; i < BLOCKSIZE; i++) {
		file->buffer[i] = 0;
	}
	for (i = file->numBlocks * BLOCKSIZE; i < (file->numBlocks + 1) * BLOCKSIZE; i++) {
		file->buffer[i] = 0;
	}
}
