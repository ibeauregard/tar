#ifndef HEADER_DATA_H
#define HEADER_DATA_H

#include "archive.h"
#include <stddef.h>
#include <sys/stat.h>
#include <stdbool.h>

typedef struct stat Stat;

typedef struct s_HeaderData {
	char name[255];
	unsigned short permissions;
	uid_t uid;
	gid_t gid;
	off_t size;
	time_t mtime;
	char type;
	char linkname[100];
	dev_t deviceNumber;
	ino_t inodeNumber;
} HeaderData;

HeaderData *getHeaderData(Stat *fileStat, char *path, HeaderData *link);
size_t getNumBlocks(const HeaderData *headerData);
int finalizeHeaderData(HeaderData *headerData);

#endif
