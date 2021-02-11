#include "tar_header.h"
#include "utils/_string.h"
#include "utils/_stdlib.h"
#if defined(__APPLE__)
#include "utils/sysmacros.h"
#else
#include <sys/sysmacros.h> // Do not remove
#endif
#include <pwd.h>
#include <grp.h>

#define BLANK 32 // ASCII value of the space char
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
static unsigned int getByteSum(const char *field, unsigned char size);
static void copyOctal(char *dest, unsigned int num, unsigned char size);

PosixHeader getZeroFilledPosixHeader()
{
	static PosixHeader header;
	return header;
}

PosixHeader *getHeaderFromData(const HeaderData *headerData, PosixHeader *header)
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
	size_t cutoff = len - (len < HEADER_NAME_SIZE - 1 ? len : HEADER_NAME_SIZE - 1);
	_strncpy(header->prefix, name, cutoff);
	_strncpy(header->name, name + cutoff, len - cutoff);
}

void setMode(const HeaderData *headerData, PosixHeader *header)
{
	copyOctal(header->mode, headerData->permissions, HEADER_MODE_SIZE);
}

void setUid(const HeaderData *headerData, PosixHeader *header)
{
	copyOctal(header->uid, headerData->uid, HEADER_UID_SIZE);
}

void setGid(const HeaderData *headerData, PosixHeader *header)
{
	copyOctal(header->gid, headerData->gid, HEADER_GID_SIZE);
}

void setSize(const HeaderData *headerData, PosixHeader *header)
{
	copyOctal(header->size, headerData->size, HEADER_SIZE_SIZE);
}

void setMtime(const HeaderData *headerData, PosixHeader *header)
{
	copyOctal(header->mtime, headerData->mtime, HEADER_MTIME_SIZE);
}

void setTypeFlagAndLinkName(const HeaderData *headerData, PosixHeader *header)
{
	header->typeflag = headerData->type;
	_strcpy(header->linkname, headerData->linkname);
}

void setMagic(PosixHeader *header)
{
	_strncpy(header->magic, TMAGIC, HEADER_MAGIC_SIZE);
}

void setVersion(PosixHeader *header)
{
	_strncpy(header->version, TVERSION, HEADER_VERSION_SIZE);
}

void setUname(const HeaderData *headerData, PosixHeader *header)
{
	_strncpy(header->uname, getpwuid(headerData->uid)->pw_name, HEADER_UNAME_SIZE);
}

void setGname(const HeaderData *headerData, PosixHeader *header)
{
	_strncpy(header->gname, getgrgid(headerData->gid)->gr_name, HEADER_GNAME_SIZE);
}

void setDevMajorDevMinor(const HeaderData *headerData, PosixHeader *header)
{
	if (header->typeflag == CHRTYPE || header->typeflag == BLKTYPE) {
		copyOctal(header->devmajor, major(headerData->deviceNumber), HEADER_DEVMAJOR_SIZE);
		copyOctal(header->devminor, minor(headerData->deviceNumber), HEADER_DEVMINOR_SIZE);
	}
}

void setChecksum(PosixHeader *header)
{
	copyOctal(header->chksum, computeChecksum(header), HEADER_CHKSUM_SIZE - 1);
    header->chksum[HEADER_CHKSUM_SIZE - 1] = BLANK;
}

unsigned int computeChecksum(PosixHeader *header)
{
	return getByteSum(header->name, HEADER_NAME_SIZE)
			+ getByteSum(header->mode, HEADER_MODE_SIZE)
			+ getByteSum(header->uid, HEADER_UID_SIZE)
			+ getByteSum(header->gid, HEADER_GID_SIZE)
			+ getByteSum(header->size, HEADER_SIZE_SIZE)
			+ getByteSum(header->mtime, HEADER_MTIME_SIZE)
			+ HEADER_CHKSUM_SIZE * BLANK
			+ header->typeflag
			+ getByteSum(header->linkname, HEADER_LINKNAME_SIZE)
			+ getByteSum(header->magic, HEADER_MAGIC_SIZE)
			+ getByteSum(header->version, HEADER_VERSION_SIZE)
			+ getByteSum(header->uname, HEADER_UNAME_SIZE)
			+ getByteSum(header->gname, HEADER_GNAME_SIZE)
			+ getByteSum(header->devmajor, HEADER_DEVMAJOR_SIZE)
			+ getByteSum(header->devminor, HEADER_DEVMINOR_SIZE)
			+ getByteSum(header->prefix, HEADER_PREFIX_SIZE);
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

char *getNameFromHeader(PosixHeader *header, char *dest)
{
	return _strcat(_strcpy(dest, header->prefix), header->name);
}

long getMtimeFromHeader(PosixHeader *header)
{
	return _strtol(header->mtime, NULL, OCTAL);
}
