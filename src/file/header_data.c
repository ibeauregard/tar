#include "header_data.h"
#include "../tar_header.h"
#include "../error/error.h"
#include "../utils/_string.h"
#if defined(__APPLE__)
#include "../utils/sysmacros.h"
#else
#include <sys/sysmacros.h>
#endif
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


#define PATH_SEP '/'
#define FILE_MODE_BITS 07777

static void setFileName(HeaderData *headerData, char *path);
static char getFileType(mode_t mode);
static void setLinkName(HeaderData *headerData);

int initHeaderData(HeaderData *headerData, char *path)
{
	Stat fileStat;
	if (lstat(path, &fileStat) == SYSCALL_ERR_CODE) {
		return error(STAT_ERR, path);
	}
	headerData->type = getFileType(fileStat.st_mode);
	setFileName(headerData, path);
	headerData->permissions = fileStat.st_mode & FILE_MODE_BITS;
	headerData->uid = fileStat.st_uid;
	headerData->gid = fileStat.st_gid;
	headerData->size = headerData->type == REGTYPE ? fileStat.st_size : 0;
	headerData->mtime = fileStat.st_mtime;
	setLinkName(headerData);
	headerData->devmajor = major(fileStat.st_dev);
	headerData->devminor = minor(fileStat.st_dev);
	return EXIT_SUCCESS;
}

char getFileType(mode_t mode)
{
	switch (mode & S_IFMT) {
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

void setFileName(HeaderData *headerData, char *path)
{
	unsigned char len = _strlen(path);
	_strncpy(headerData->name, path, 255);
	if (headerData->type != DIRTYPE) {
		return;
	}
	unsigned char numSlashes;
	for (numSlashes = 0; numSlashes < len && path[len - numSlashes - 1] == PATH_SEP; numSlashes++);
	if (numSlashes == 0) {
		headerData->name[len] = PATH_SEP;
	}
	headerData->name[len - numSlashes + 1] = 0;
}

void setLinkName(HeaderData *headerData)
{
	if (headerData->type == SYMTYPE) {
		readlink(headerData->name, headerData->linkname, 100);
	} else {
		for (int i = 0; i < 100; i++) {
			headerData->linkname[i] = 0;
		}
	}
}

size_t getNumBlocks(const HeaderData *headerData)
{
	if (headerData->type != REGTYPE) {
		return 0;
	}
	return headerData->size ? (headerData->size - 1) / BLOCKSIZE + 1 : 0;
}

int finalizeHeaderData(HeaderData *headerData)
{
	free(headerData);
	return EXIT_SUCCESS;
}
