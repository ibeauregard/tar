#include "header_data.h"
#include "../tar_header.h"
#include "../error/error.h"
#include "../utils/_string.h"
#include <stdlib.h>
#include <fcntl.h>

#define PATH_SEP '/'

static char *getFilePath(HeaderData *headerData, char *path);
static char getFileType(const HeaderData *headerData);
static size_t getNumBlocks(const HeaderData *headerData);

int initHeaderData(HeaderData *headerData, char *path)
{
	if (lstat(path, &headerData->fileStat) == SYSCALL_ERR_CODE) {
		return error(STAT_ERR, path);
	}
	headerData->type = getFileType(headerData);
	headerData->path = getFilePath(headerData, path);
	headerData->numBlocks = getNumBlocks(headerData);
	return EXIT_SUCCESS;
}

char getFileType(const HeaderData *headerData)
{
	switch (headerData->fileStat.st_mode & S_IFMT) {
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

char* getFilePath(HeaderData *headerData, char *path)
{
	unsigned char len = _strlen(path);
	char *newPath = malloc(len + 2);
	_strcpy(newPath, path);
	if (headerData->type != DIRTYPE) {
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

size_t getNumBlocks(const HeaderData *headerData)
{
	if (headerData->type != REGTYPE) {
		return 0;
	}
	return headerData->fileStat.st_size ? (headerData->fileStat.st_size - 1) / BLOCKSIZE + 1 : 0;
}

int finalizeHeaderData(HeaderData *headerData)
{
	free(headerData->path);
	free(headerData);
	return EXIT_SUCCESS;
}
