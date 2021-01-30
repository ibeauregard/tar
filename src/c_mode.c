#include "modes.h"
#include "tar_header.h"
#include "utils/_string.h"
#include "utils/_stdio.h"
#include "constants.h"
#include "archived_file.h"
#include "archive_file.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#define ARCHIVE_FLAGS O_CREAT|O_WRONLY|O_TRUNC
#define ARCHIVE_MODE S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
#define END_OF_ARCHIVE_SIZE 2 * BLOCKSIZE
#define STAT_ERR "my_tar: %s: Cannot stat: No such file or directory\n"
#define CANT_OPEN_FILE_ERR "my_tar: Cannot open %s\n"
#define CANT_READ_ERR "my_tar: Cannot read from %s\n"
#define CANT_WRITE_ERR "my_tar: Cannot write to %s\n"
#define FILE_IS_ARCHIVE_ERR "my_tar: %s: file is the archive; not dumped\n"

typedef struct stat Stat;

static int getArchiveFd(const char *archivePath);
static int append(ArchiveFile *archiveFile, const char *path);
//static int write_header(ArchiveFile *archiveFile);//, const Stat *fileStat);
static int pushContent(ArchivedFile *file, ArchiveFile *archiveFile);
static int appendEnd(ArchiveFile *archiveFile);
static blkcnt_t getNumBlocks(off_t numBytes);
static int error(const char *message, const char *messageArg);

int c_mode(Params *params)
{
	ArchiveFile archiveFile = {
			.path = params->archivePath,
			.fd = getArchiveFd(params->archivePath),
	};
	if (archiveFile.fd == SYSCALL_ERR_CODE) {
		return error(CANT_OPEN_FILE_ERR, params->archivePath);
	}
	while (params->filePaths) {
		if (append(&archiveFile, params->filePaths->path)) return EXIT_FAILURE;
		PathNode *current = params->filePaths;
		params->filePaths = params->filePaths->next;
		free(current);
	}
	appendEnd(&archiveFile);
	close(archiveFile.fd);
	return EXIT_SUCCESS;
}

int getArchiveFd(const char *archivePath)
{
	if (_strcmp(archivePath, STDOUT_PATH)) {
		return open(archivePath, ARCHIVE_FLAGS, ARCHIVE_MODE);
	}
	return STDOUT_FILENO;
}

int append(ArchiveFile *archiveFile, const char *path)
{
	Stat fileStat;
	if (lstat(path, &fileStat) == SYSCALL_ERR_CODE) {
		return error(STAT_ERR, path);
	}
	ArchivedFile *file = newArchivedFile(path, getNumBlocks(fileStat.st_size));
//	write_header(archiveFile);//, &fileStat);
	pushContent(file, archiveFile);
	if (writeToArchive(file, archiveFile) == SYSCALL_ERR_CODE) {
		return error(CANT_WRITE_ERR, archiveFile->path);
	}
	freeArchivedFile(file);
	return EXIT_SUCCESS;
}

//int write_header(ArchiveFile *archiveFile)//, const Stat *fileStat)
//{
//	PosixHeader header = {};
//	write(archiveFile->fd, &header, BLOCKSIZE);
//	zfill(archiveFile->block);
//	return EXIT_SUCCESS;
//}

int pushContent(ArchivedFile *file, ArchiveFile *archiveFile)
{
	if (!_strcmp(archiveFile->path, file->path)) {
		return error(FILE_IS_ARCHIVE_ERR, file->path);
	}
	int fd = open(file->path, O_RDONLY);
	if (fd == SYSCALL_ERR_CODE) {
		return error(CANT_OPEN_FILE_ERR, file->path);
	}
	if (readFromFd(file, fd) == SYSCALL_ERR_CODE) {
		return error(CANT_READ_ERR, file->path);
	}
	close(fd);
	return EXIT_SUCCESS;
}

int appendEnd(ArchiveFile *archiveFile)
{
	char blocks[END_OF_ARCHIVE_SIZE] = {0};
	if (write(archiveFile->fd, blocks, END_OF_ARCHIVE_SIZE) == SYSCALL_ERR_CODE) {
		return error(CANT_WRITE_ERR, archiveFile->path);
	}
	return EXIT_SUCCESS;
}

inline blkcnt_t getNumBlocks(off_t numBytes)
{
	return (numBytes - 1) / BLOCKSIZE + 1;
}

int error(const char *message, const char *messageArg)
{
	_dprintf(STDERR_FILENO, message, messageArg);
	return EXIT_FAILURE;
}
