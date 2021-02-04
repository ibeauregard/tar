#include "tar_header.h"
#include "utils/_string.h"
#if defined(__APPLE__)
#include "utils/sysmacros.h"
#else
#include <sys/sysmacros.h>
#endif
#include <pwd.h>
#include <grp.h>

#define NAME_SIZE 100
#define DIGITS "01234567"
#define OCTAL 8

static void setNameAndPrefix(const HeaderData *headerData, PosixHeader *header);
static void setMode(const HeaderData *headerData, PosixHeader *header);
static void setUid(const HeaderData *headerData, PosixHeader *header);
static void setGid(const HeaderData *headerData, PosixHeader *header);
static void setSize(const HeaderData *headerData, PosixHeader *header);
static void setMtime(const HeaderData *headerData, PosixHeader *header);
static void setTypeFlagAndLinkName(const HeaderData *headerData, PosixHeader *header);
static void setMagic(PosixHeader *header);
static void setVersion(PosixHeader *header);
static void setUname(const HeaderData *headerData, PosixHeader *header);
static void setGname(const HeaderData *headerData, PosixHeader *header);
static void setDevMajorDevMinor(const HeaderData *headerData, PosixHeader *header);
static void setChecksum(PosixHeader *header);
static unsigned int computeChecksum(PosixHeader *header);
static unsigned int getByteSum(const char *field, unsigned char size);
static void copyOctal(char *dest, unsigned int num, unsigned char size);

PosixHeader getZeroFilledPosixHeader()
{
	static PosixHeader header;
	return header;
}

PosixHeader *getFilledHeader(const HeaderData *headerData, PosixHeader *header)
{
	setNameAndPrefix(headerData, header);
	setMode(headerData, header);
	setUid(headerData, header);
	setGid(headerData, header);
	setSize(headerData, header);
	setMtime(headerData, header);
	setTypeFlagAndLinkName(headerData, header);
	setMagic(header);
	setVersion(header);
	setUname(headerData, header);
	setGname(headerData, header);
	setDevMajorDevMinor(headerData, header);

	setChecksum(header);
	return header;
}

void setNameAndPrefix(const HeaderData *headerData, PosixHeader *header)
{
	const char *name = headerData->name;
	size_t len = _strlen(name);
	size_t cutoff = len - (len < NAME_SIZE - 1 ? len : NAME_SIZE - 1);
	_strncpy(header->prefix, name, cutoff);
	_strncpy(header->name, name + cutoff, len - cutoff);
}

void setMode(const HeaderData *headerData, PosixHeader *header)
{
	copyOctal(header->mode, headerData->permissions, 8);
}

void setUid(const HeaderData *headerData, PosixHeader *header)
{
	copyOctal(header->uid, headerData->uid, 8);
}

void setGid(const HeaderData *headerData, PosixHeader *header)
{
	copyOctal(header->gid, headerData->gid, 8);
}

void setSize(const HeaderData *headerData, PosixHeader *header)
{
	copyOctal(header->size, headerData->size, 12);
}

void setMtime(const HeaderData *headerData, PosixHeader *header)
{
	copyOctal(header->mtime, headerData->mtime, 12);
}

void setTypeFlagAndLinkName(const HeaderData *headerData, PosixHeader *header)
{
	header->typeflag = headerData->type;
	_strcpy(header->linkname, headerData->linkname);
}

void setMagic(PosixHeader *header)
{
	_strncpy(header->magic, TMAGIC, TMAGLEN);
}

void setVersion(PosixHeader *header)
{
	_strncpy(header->version, TVERSION, TVERSLEN);
}

void setUname(const HeaderData *headerData, PosixHeader *header)
{
	_strncpy(header->uname, getpwuid(headerData->uid)->pw_name, 32);
}

void setGname(const HeaderData *headerData, PosixHeader *header)
{
	_strncpy(header->gname, getgrgid(headerData->gid)->gr_name, 32);
}

void setDevMajorDevMinor(const HeaderData *headerData, PosixHeader *header)
{
	if (header->typeflag == CHRTYPE || header->typeflag == BLKTYPE) {
		copyOctal(header->devmajor, major(headerData->deviceNumber), 8);
		copyOctal(header->devminor, minor(headerData->deviceNumber), 8);
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
