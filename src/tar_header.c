#include "tar_header.h"
#include "utils/_string.h"
#if defined(__APPLE__)
#include "utils/sysmacros.h"
#else
#include <sys/sysmacros.h>
#endif
#include <pwd.h>
#include <grp.h>

#define NAME_LENGTH 100
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
static void setMagic(PosixHeader *header);
static void setVersion(PosixHeader *header);
static void setUname(const ArchivedFile *file, PosixHeader *header);
static void setGname(const ArchivedFile *file, PosixHeader *header);
static void setDevMajorDevMinor(const ArchivedFile *file, PosixHeader *header);
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
	setMagic(header);
	setVersion(header);
	setUname(file, header);
	setGname(file, header);
	setDevMajorDevMinor(file, header);
}

void setNameAndPrefix(const ArchivedFile *file, PosixHeader *header)
{
	const char *name = file->path;
	size_t len = _strlen(name);
	size_t cutoff = len - (len < NAME_LENGTH - 1 ? len : NAME_LENGTH - 1);
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
			header->typeflag = SYMTYPE;
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

void setMagic(PosixHeader *header)
{
	_strncpy(header->magic, TMAGIC, TMAGLEN);
}

void setVersion(PosixHeader *header)
{
	_strncpy(header->version, TVERSION, TVERSLEN);
}

void setUname(const ArchivedFile *file, PosixHeader *header)
{
	_strcpy(header->uname, getpwuid(file->fileStat->st_uid)->pw_name);
}

void setGname(const ArchivedFile *file, PosixHeader *header)
{
	_strcpy(header->gname, getgrgid(file->fileStat->st_gid)->gr_name);
}

void setDevMajorDevMinor(const ArchivedFile *file, PosixHeader *header)
{
	_itoa(header->devmajor, major(file->fileStat->st_dev), 8, OCTAL);
	_itoa(header->devminor, minor(file->fileStat->st_dev), 8, OCTAL);
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
