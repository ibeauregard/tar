#include "header_data.h"
#include "tar_header.h"
#include "utils/_string.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


#define PATH_SEP '/'
#define FILE_MODE_BITS 07777

HeaderData *fromStatAndPath(Stat *fileStat, char *path);
HeaderData *fromLinkAndPath(HeaderData *link, char *path);
static void setFileName(HeaderData *headerData, char *path);
static char getFileType(mode_t mode);
static void setLinkName(HeaderData *headerData, char *name);

HeaderData *getHeaderData(Stat *fileStat, char *path, HeaderData *link)
{
	if (link) {
		return fromLinkAndPath(link, path);
	}
	return fromStatAndPath(fileStat, path);
}

HeaderData *fromStatAndPath(Stat *fileStat, char *path)
{
	HeaderData *headerData = malloc(sizeof (HeaderData));
	headerData->type = getFileType(fileStat->st_mode);
	setFileName(headerData, path);
	headerData->permissions = fileStat->st_mode & FILE_MODE_BITS;
	headerData->uid = fileStat->st_uid;
	headerData->gid = fileStat->st_gid;
	headerData->size = headerData->type == REGTYPE ? fileStat->st_size : 0;
	headerData->mtime = fileStat->st_mtime;
	setLinkName(headerData, headerData->name);
	headerData->deviceNumber = fileStat->st_dev;
	headerData->inodeNumber = fileStat->st_ino;
	return headerData;
}

HeaderData *fromLinkAndPath(HeaderData *link, char *path)
{
	HeaderData *headerData = malloc(sizeof (HeaderData));
	headerData->type = LNKTYPE;
	_strncpy(headerData->name, path, HEADER_DATA_NAME_SIZE);
	headerData->permissions = link->permissions;
	headerData->uid = link->uid;
	headerData->gid = link->gid;
	headerData->size = 0;
	headerData->mtime = link->mtime;
	setLinkName(headerData, link->name);
	headerData->deviceNumber = link->deviceNumber;
	headerData->inodeNumber = link->inodeNumber;
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
	_strncpy(headerData->name, path, HEADER_DATA_NAME_SIZE);
	if (headerData->type != DIRTYPE) {
		return;
	}
	unsigned char len = _strlen(path);
	unsigned char numSlashes;
	for (numSlashes = 0; numSlashes < len && path[len - numSlashes - 1] == PATH_SEP; numSlashes++);
	headerData->name[len - numSlashes] = PATH_SEP;
	headerData->name[len - numSlashes + 1] = 0;
}

void setLinkName(HeaderData *headerData, char *name)
{
	ssize_t i = 0;
	if (headerData->type == SYMTYPE) {
		i = readlink(name, headerData->linkname, HEADER_DATA_LINKNAME_SIZE);
	} else if (headerData->type == LNKTYPE) {
		unsigned int nameLength = _strlen(name);
		i = nameLength > HEADER_DATA_LINKNAME_SIZE - 1 ? HEADER_DATA_LINKNAME_SIZE - 1 : nameLength;
		_strncpy(headerData->linkname, name, i);
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
