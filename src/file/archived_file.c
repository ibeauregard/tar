#include "archived_file.h"
#include "../tar_header.h"
#include "unistd.h"
#include "../error/error.h"
#include <stdlib.h>
#include <fcntl.h>

static void zfill(ArchivedFile *file);

int initArchivedFile(ArchivedFile *file, const char *path, size_t numBlocks)
{
	file->fd = open(path, O_RDONLY);
	if (file->fd == SYSCALL_ERR_CODE) {
		return error(CANT_OPEN_FILE_ERR, path);
	}
	file->path = path;
	file->buffer = malloc(numBlocks * BLOCKSIZE);
	file->numBlocks = numBlocks;
	zfill(file);
	return EXIT_SUCCESS;
}

int readFile(ArchivedFile *file)
{
	return read(file->fd, file->buffer, file->numBlocks * BLOCKSIZE);
}

int writeToArchive(ArchivedFile *file, Archive *archive)
{
	return write(archive->fd, file->buffer, file->numBlocks * BLOCKSIZE);
}

int closeArchivedFile(ArchivedFile *file)
{
	free(file->buffer);
	return close(file->fd);
}

void zfill(ArchivedFile *file)
{
	size_t totalBytes = file->numBlocks * BLOCKSIZE;
	for (size_t i = (file->numBlocks - 1) * BLOCKSIZE; i < totalBytes; i++) {
		file->buffer[i] = 0;
	}
}
