#include "archived_file.h"
#include "../tar_header.h"
#include "unistd.h"
#include "../error/error.h"
#include "../utils/_string.h"
#include <stdlib.h>
#include <fcntl.h>

#define PATH_SEP '/'

static char *getFilePath(ArchivedFile *file, char *path);
static void zfillLastBlock(ArchivedFile *file);
static char getFileType(const ArchivedFile *file);
static size_t getNumBlocks(const ArchivedFile *file);

int initArchivedFile(ArchivedFile *file, char *path)
{
	Stat *fileStat = malloc(sizeof (Stat));
	if (lstat(path, fileStat) == SYSCALL_ERR_CODE) {
		free(fileStat);
		return error(STAT_ERR, path);
	}
	file->fileStat = fileStat;
	file->fd = open(path, O_RDONLY);
	if (file->fd == SYSCALL_ERR_CODE) {
		free(fileStat);
		return error(CANT_OPEN_FILE_ERR, path);
	}
	file->type = getFileType(file);
	file->path = getFilePath(file, path);
	file->numBlocks = getNumBlocks(file);
	file->buffer = malloc(file->numBlocks * BLOCKSIZE);
	zfillLastBlock(file);
	return EXIT_SUCCESS;
}

char getFileType(const ArchivedFile *file)
{
	switch (file->fileStat->st_mode & S_IFMT) {
		case S_IFREG:
			return REGTYPE;
		case S_IFLNK:
			return SYMTYPE;
		case S_IFCHR:
			return CHRTYPE;
		case S_IFBLK:
			return BLKTYPE;
		case S_IFDIR:
			return DIRTYPE;
		case S_IFIFO:
			return FIFOTYPE;
		default:
			return REGTYPE;
	}
}

char* getFilePath(ArchivedFile *file, char *path)
{
	unsigned char len = _strlen(path);
	char *newPath = malloc(len + 2);
	_strcpy(newPath, path);
	if (file->type != DIRTYPE) {
		return newPath;
	}
	unsigned char numSlashes;
	for (numSlashes = 0; numSlashes < len && path[len - numSlashes - 1] == PATH_SEP; numSlashes++);
	if (numSlashes == 0) {
		newPath[len] = PATH_SEP;
	}
	newPath[len - numSlashes + 1] = 0;
	return newPath;
}

size_t getNumBlocks(const ArchivedFile *file)
{
	if (file->type != REGTYPE) {
		return 0;
	}
	return file->fileStat->st_size ? (file->fileStat->st_size - 1) / BLOCKSIZE + 1 : 0;
}

int readFile(const ArchivedFile *file)
{
	if (file->numBlocks == 0) {
		return EXIT_SUCCESS;
	}
	return read(file->fd, file->buffer, file->numBlocks * BLOCKSIZE);
}

int writeToArchive(const ArchivedFile *file, Archive *archive)
{
	return write(archive->fd, file->buffer, file->numBlocks * BLOCKSIZE);
}

int finalizeArchivedFile(const ArchivedFile *file)
{
	free(file->fileStat);
	free(file->buffer);
	free(file->path);
	return close(file->fd);
}

void zfillLastBlock(ArchivedFile *file)
{
	size_t totalBytes = file->numBlocks * BLOCKSIZE;
	for (size_t i = (file->numBlocks - 1) * BLOCKSIZE; i < totalBytes; i++) {
		file->buffer[i] = 0;
	}
}
