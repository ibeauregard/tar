#include "header_data.h"
#include "tar_header.h"
#include "utils/_string.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


#define PATH_SEP '/'
#define FILE_MODE_BITS 07777

static void setFileName(HeaderData *headerData, char *path);
static char getFileType(mode_t mode);
static void setLinkName(HeaderData *headerData);

HeaderData *fromStatAndPath(Stat *fileStat, char *path, bool previouslyListed)
{
	HeaderData *headerData = malloc(sizeof (HeaderData));
	headerData->type = previouslyListed ? LNKTYPE : getFileType(fileStat->st_mode);
	setFileName(headerData, path);
	headerData->permissions = fileStat->st_mode & FILE_MODE_BITS;
	headerData->uid = fileStat->st_uid;
	headerData->gid = fileStat->st_gid;
	headerData->size = headerData->type == REGTYPE ? fileStat->st_size : 0;
	headerData->mtime = fileStat->st_mtime;
	setLinkName(headerData);
	headerData->deviceNumber = fileStat->st_dev;
	headerData->inodeNumber = fileStat->st_ino;
	return headerData;
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
	int i = 0;
	if (headerData->type == SYMTYPE) {
		i = readlink(headerData->name, headerData->linkname, 100);
	} else if (headerData->type == LNKTYPE) {
		unsigned int nameLength = _strlen(headerData->name);
		i = nameLength > 99 ? 99 : nameLength;
		_strncpy(headerData->linkname, headerData->name, i);
	}
	headerData->linkname[i] = 0;

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
