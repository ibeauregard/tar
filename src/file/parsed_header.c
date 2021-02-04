#include "parsed_header.h"
#include "../tar_header.h"
#include "../error/error.h"
#include "../utils/_string.h"
#include <stdlib.h>
#include <fcntl.h>

#define PATH_SEP '/'

static char *getFilePath(ParsedHeader *parsedHeader, char *path);
static char getFileType(const ParsedHeader *parsedHeader);
static size_t getNumBlocks(const ParsedHeader *parsedHeader);

int initParsedHeader(ParsedHeader *parsedHeader, char *path)
{
	Stat *fileStat = malloc(sizeof (Stat));
	if (lstat(path, fileStat) == SYSCALL_ERR_CODE) {
		return error(STAT_ERR, path);
	}
	parsedHeader->fileStat = fileStat;
	parsedHeader->type = getFileType(parsedHeader);
	parsedHeader->path = getFilePath(parsedHeader, path);
	parsedHeader->numBlocks = getNumBlocks(parsedHeader);
	return EXIT_SUCCESS;
}

char getFileType(const ParsedHeader *parsedHeader)
{
	switch (parsedHeader->fileStat->st_mode & S_IFMT) {
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

char* getFilePath(ParsedHeader *parsedHeader, char *path)
{
	unsigned char len = _strlen(path);
	char *newPath = malloc(len + 2);
	_strcpy(newPath, path);
	if (parsedHeader->type != DIRTYPE) {
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

size_t getNumBlocks(const ParsedHeader *parsedHeader)
{
	if (parsedHeader->type != REGTYPE) {
		return 0;
	}
	return parsedHeader->fileStat->st_size ? (parsedHeader->fileStat->st_size - 1) / BLOCKSIZE + 1 : 0;
}

int finalizeParsedHeader(ParsedHeader *parsedHeader)
{
	free(parsedHeader->fileStat);
	free(parsedHeader->path);
	free(parsedHeader);
	return EXIT_SUCCESS;
}
