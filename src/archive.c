#include "archive.h"
#include "utils/_string.h"
#include "fcntl.h"
#include "constants.h"
#include "error/error.h"
#include "tar_header.h"
#include <unistd.h>
#include <stdlib.h>

#define APPEND_FLAGS O_CREAT|O_RDWR
#define TRUNCATE_FLAGS APPEND_FLAGS|O_TRUNC
#define ARCHIVE_MODE S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
#define END_OF_ARCHIVE_SIZE 2 * BLOCKSIZE

static int getArchiveFd(const char *archivePath, bool append);
static int getArchiveFlags(bool append);

int initArchive(Archive *archive, const char *archivePath, bool append)
{
	archive->fd = getArchiveFd(archivePath, append);
	if (archive->fd == SYSCALL_ERR_CODE) {
		return error(CANT_OPEN_FILE_ERR, archivePath);
	}
	archive->path = archivePath;
	return EXIT_SUCCESS;
}

int appendEnd(Archive *archive)
{
	char blocks[END_OF_ARCHIVE_SIZE] = {0};
	if (write(archive->fd, blocks, END_OF_ARCHIVE_SIZE) == SYSCALL_ERR_CODE) {
		return error(CANT_WRITE_ERR, archive->path);
	}
	return EXIT_SUCCESS;
}

int finalizeArchive(Archive *archive)
{
	return close(archive->fd);
}

int getArchiveFd(const char *archivePath, bool append)
{
	if (_strcmp(archivePath, STDOUT_PATH)) {
		return open(archivePath, getArchiveFlags(append), ARCHIVE_MODE);
	}
	return STDOUT_FILENO;
}

inline int getArchiveFlags(bool append)
{
	return append ? APPEND_FLAGS : TRUNCATE_FLAGS;
}
