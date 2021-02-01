#include "archived_file.h"
#include "../tar_header.h"
#include "unistd.h"
#include "../error/error.h"
#include <stdlib.h>
#include <fcntl.h>

static void zfillLastBlock(ArchivedFile *file);

int initArchivedFile(ArchivedFile *file, const char *path)
{
	Stat *fileStat = malloc(sizeof (Stat));
	if (lstat(path, fileStat) == SYSCALL_ERR_CODE) {
		return error(STAT_ERR, path);
	}
	file->fileStat = fileStat;
	file->fd = open(path, O_RDONLY);
	if (file->fd == SYSCALL_ERR_CODE) {
		return error(CANT_OPEN_FILE_ERR, path);
	}
	file->path = path;
	file->numBlocks = (fileStat->st_size - 1) / BLOCKSIZE + 1;
	file->buffer = malloc(file->numBlocks * BLOCKSIZE);
	zfillLastBlock(file);
	return EXIT_SUCCESS;
}

int readFile(const ArchivedFile *file)
{
	return read(file->fd, file->buffer, file->numBlocks * BLOCKSIZE);
}

int writeToArchive(const ArchivedFile *file, Archive *archive)
{
	return write(archive->fd, file->buffer, file->numBlocks * BLOCKSIZE);
}

int closeArchivedFile(const ArchivedFile *file)
{
	free(file->fileStat);
	free(file->buffer);
	return close(file->fd);
}

void zfillLastBlock(ArchivedFile *file)
{
	size_t totalBytes = file->numBlocks * BLOCKSIZE;
	for (size_t i = (file->numBlocks - 1) * BLOCKSIZE; i < totalBytes; i++) {
		file->buffer[i] = 0;
	}
}
