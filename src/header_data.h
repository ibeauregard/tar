#ifndef HEADER_DATA_H
#define HEADER_DATA_H

#include "archive.h"
#include <stddef.h>
#include <sys/stat.h>
#include <stdbool.h>

#define HEADER_DATA_NAME_SIZE 255
#define HEADER_DATA_LINKNAME_SIZE 100

typedef struct stat Stat;

typedef struct s_HeaderData {
	char name[HEADER_DATA_NAME_SIZE];
	unsigned short permissions;
	uid_t uid;
	gid_t gid;
	off_t size;
	time_t mtime;
	char type;
	char linkname[HEADER_DATA_LINKNAME_SIZE];
	dev_t deviceNumber;
	ino_t inodeNumber;
} HeaderData;

HeaderData *getHeaderData(Stat *fileStat, char *path, HeaderData *link);
size_t getNumBlocks(const HeaderData *headerData);
int finalizeHeaderData(HeaderData *headerData);

#endif
