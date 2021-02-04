#include "tar_header.h"
#include "utils/_string.h"
#if defined(__APPLE__)
#include "utils/sysmacros.h"
#else
#include <sys/sysmacros.h>
#endif
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>

#define NAME_SIZE 100
#define DIGITS "01234567"
#define OCTAL 8
#define FILE_MODE_BITS 07777

static void setNameAndPrefix(const ParsedHeader *parsedHeader, PosixHeader *header);
static void setMode(const ParsedHeader *parsedHeader, PosixHeader *header);
static void setUid(const ParsedHeader *parsedHeader, PosixHeader *header);
static void setGid(const ParsedHeader *parsedHeader, PosixHeader *header);
static void setSize(const ParsedHeader *parsedHeader, PosixHeader *header);
static void setMtime(const ParsedHeader *parsedHeader, PosixHeader *header);
static void setTypeFlagAndLinkName(const ParsedHeader *parsedHeader, PosixHeader *header);
static void setMagic(PosixHeader *header);
static void setVersion(PosixHeader *header);
static void setUname(const ParsedHeader *parsedHeader, PosixHeader *header);
static void setGname(const ParsedHeader *parsedHeader, PosixHeader *header);
static void setDevMajorDevMinor(const ParsedHeader *parsedHeader, PosixHeader *header);
static void setChecksum(PosixHeader *header);
static unsigned int computeChecksum(PosixHeader *header);
static unsigned int getByteSum(const char *field, unsigned char size);
static void copyOctal(char *dest, unsigned int num, unsigned char size);

PosixHeader getZeroFilledPosixHeader()
{
	static PosixHeader header;
	return header;
}

PosixHeader *getFilledHeader(const ParsedHeader *parsedHeader, PosixHeader *header)
{
	setNameAndPrefix(parsedHeader, header);
	setMode(parsedHeader, header);
	setUid(parsedHeader, header);
	setGid(parsedHeader, header);
	setSize(parsedHeader, header);
	setMtime(parsedHeader, header);
	setTypeFlagAndLinkName(parsedHeader, header);
	setMagic(header);
	setVersion(header);
	setUname(parsedHeader, header);
	setGname(parsedHeader, header);
	setDevMajorDevMinor(parsedHeader, header);

	setChecksum(header);
	return header;
}

void setNameAndPrefix(const ParsedHeader *parsedHeader, PosixHeader *header)
{
	const char *name = parsedHeader->path;
	size_t len = _strlen(name);
	size_t cutoff = len - (len < NAME_SIZE - 1 ? len : NAME_SIZE - 1);
	_strncpy(header->prefix, name, cutoff);
	_strncpy(header->name, name + cutoff, len - cutoff);
}

void setMode(const ParsedHeader *parsedHeader, PosixHeader *header)
{
	copyOctal(header->mode, parsedHeader->fileStat->st_mode & FILE_MODE_BITS, 8);
}

void setUid(const ParsedHeader *parsedHeader, PosixHeader *header)
{
	copyOctal(header->uid, parsedHeader->fileStat->st_uid, 8);
}

void setGid(const ParsedHeader *parsedHeader, PosixHeader *header)
{
	copyOctal(header->gid, parsedHeader->fileStat->st_gid, 8);
}

void setSize(const ParsedHeader *parsedHeader, PosixHeader *header)
{
	copyOctal(header->size,
			  parsedHeader->type == REGTYPE ? parsedHeader->fileStat->st_size : 0,
			  12);
}

void setMtime(const ParsedHeader *parsedHeader, PosixHeader *header)
{
	copyOctal(header->mtime, parsedHeader->fileStat->st_mtime, 12);
}

void setTypeFlagAndLinkName(const ParsedHeader *parsedHeader, PosixHeader *header)
{
	header->typeflag = parsedHeader->type;
	if (header->typeflag == SYMTYPE) {
		readlink(parsedHeader->path, header->linkname, 100);
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

void setUname(const ParsedHeader *parsedHeader, PosixHeader *header)
{
	_strncpy(header->uname, getpwuid(parsedHeader->fileStat->st_uid)->pw_name, 32);
}

void setGname(const ParsedHeader *parsedHeader, PosixHeader *header)
{
	_strncpy(header->gname, getgrgid(parsedHeader->fileStat->st_gid)->gr_name, 32);
}

void setDevMajorDevMinor(const ParsedHeader *parsedHeader, PosixHeader *header)
{
	if (header->typeflag == CHRTYPE || header->typeflag == BLKTYPE) {
		copyOctal(header->devmajor, major(parsedHeader->fileStat->st_dev), 8);
		copyOctal(header->devminor, minor(parsedHeader->fileStat->st_dev), 8);
	}
}

void setChecksum(PosixHeader *header)
{
	copyOctal(header->chksum, computeChecksum(header), 7);
    header->chksum[7] = ' ';
}

unsigned int computeChecksum(PosixHeader *header)
{
	return getByteSum(header->name, 100)
			+ getByteSum(header->mode, 8)
			+ getByteSum(header->uid, 8)
			+ getByteSum(header->gid, 8)
			+ getByteSum(header->size, 12)
			+ getByteSum(header->mtime, 12)
			+ 8 * ' ' // checksum field
			+ header->typeflag
			+ getByteSum(header->linkname, 100)
			+ getByteSum(header->magic, 6)
			+ getByteSum(header->version, 2)
			+ getByteSum(header->uname, 32)
			+ getByteSum(header->gname, 32)
			+ getByteSum(header->devmajor, 8)
			+ getByteSum(header->devminor, 8)
			+ getByteSum(header->prefix, 155);
}

unsigned int getByteSum(const char *field, unsigned char size)
{
	unsigned int sum = 0;
	for (unsigned char i = 0; i < size && field[i]; i++) {
		sum += field[i];
	}
	return sum;
}

void copyOctal(char *dest, unsigned int num, unsigned char size)
{
	unsigned char i;
	for (i = 0; i < size - 1; i++) {
		dest[i] = '0';
	}
	dest[i] = 0;
	while (num) {
		dest[--i] = DIGITS[num % OCTAL];
		num /= OCTAL;
	}
}
