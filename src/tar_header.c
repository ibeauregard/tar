#include "tar_header.h"
#include "utils/_string.h"
#include <stdio.h>

#define DIGITS "01234567"
#define OCTAL 8
#define FILE_MODE_BITS 07777

static void setNameAndPrefix(const ArchivedFile *file, PosixHeader *header);
static void setMode(const ArchivedFile *file, PosixHeader *header);
static void setUid(const ArchivedFile *file, PosixHeader *header);
static void setGid(const ArchivedFile *file, PosixHeader *header);
static void setSize(const ArchivedFile *file, PosixHeader *header);
static void setMtime(const ArchivedFile *file, PosixHeader *header);
static void setTypeFlag(const ArchivedFile *file, PosixHeader *header);
static void _itoa(char *dest, unsigned int num, unsigned char size, unsigned char base);

void fillHeader(const ArchivedFile *file, PosixHeader *header)
{
	setNameAndPrefix(file, header);
	setMode(file, header);
	setUid(file, header);
	setGid(file, header);
	setSize(file, header);
	setMtime(file, header);
	setTypeFlag(file, header);
}

void setNameAndPrefix(const ArchivedFile *file, PosixHeader *header)
{
	const char *name = file->path;
	size_t len = _strlen(name);
	size_t cutoff = len - (len < 99 ? len : 99);
	_strncpy(header->prefix, name, cutoff);
	_strncpy(header->name, name + cutoff, len - cutoff);
}

void setMode(const ArchivedFile *file, PosixHeader *header)
{
	_itoa(header->mode, file->fileStat->st_mode & FILE_MODE_BITS, 8, OCTAL);
}

void setUid(const ArchivedFile *file, PosixHeader *header)
{
	_itoa(header->uid, file->fileStat->st_uid, 8, OCTAL);
}

void setGid(const ArchivedFile *file, PosixHeader *header)
{
	_itoa(header->gid, file->fileStat->st_gid, 8, OCTAL);
}

void setSize(const ArchivedFile *file, PosixHeader *header)
{
	_itoa(header->size, file->fileStat->st_size, 12, OCTAL);
}

void setMtime(const ArchivedFile *file, PosixHeader *header)
{
	_itoa(header->mtime, file->fileStat->st_mtime, 12, OCTAL);
}

void setTypeFlag(const ArchivedFile *file, PosixHeader *header)
{
	switch (file->fileStat->st_mode & S_IFMT) {
		case S_IFREG:
			header->typeflag = REGTYPE;
			break;
		case S_IFLNK:
			header->typeflag = LNKTYPE;
			break;
		case S_IFCHR:
			header->typeflag = CHRTYPE;
			break;
		case S_IFBLK:
			header->typeflag = BLKTYPE;
			break;
		case S_IFDIR:
			header->typeflag = DIRTYPE;
			break;
		case S_IFIFO:
			header->typeflag = FIFOTYPE;
			break;
	}
}

void _itoa(char *dest, unsigned int num, unsigned char size, unsigned char base)
{
	unsigned char i;
	for (i = 0; i < size - 1; i++) {
		dest[i] = '0';
	}
	dest[i] = 0;
	while (num) {
		dest[--i] = DIGITS[num % base];
		num /= base;
	}
}
