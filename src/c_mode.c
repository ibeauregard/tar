#include "modes.h"
#include "tar_header.h"
#include "utils/_string.h"
#include "utils/_stdio.h"
#include "constants.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#define ARCHIVE_FLAGS O_CREAT|O_WRONLY|O_TRUNC
#define ARCHIVE_MODE S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
#define SYSCALL_ERR_CODE -1
#define END_OF_ARCHIVE_N_BLOCKS 2
#define STAT_ERR "my_tar: %s: Cannot stat: No such file or directory\n"
#define CANT_OPEN_FILE_ERR "my_tar: Cannot open %s\n"
#define CANT_READ_ERR "my_tar: Cannot read from %s\n"
#define CANT_WRITE_ERR "my_tar: Cannot write to %s\n"
#define FILE_IS_ARCHIVE_ERR "my_tar: %s: file is the archive; not dumped\n"

typedef struct stat Stat;
typedef struct s_archive_file {
	char *path;
	int fd;
	char block[BLOCKSIZE];
} ArchiveFile;

static int getArchiveFd(const char *archivePath);
static int append(ArchiveFile *archiveFile, const char *path);
//static int write_header(int archiveFD, Stat *fileStat, char *block);
static int writeContent(ArchiveFile *archiveFile, const char *path, blkcnt_t n_blocks);
static int appendEnd(ArchiveFile *archiveFile);
static void zfill(char *block);
static blkcnt_t getNumBlocks(off_t n_bytes);
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
//	write_header(archiveFD, &fileStat, block);
	writeContent(archiveFile, path, getNumBlocks(fileStat.st_size));
	return EXIT_SUCCESS;
}

//int write_header(int archiveFD, Stat *fileStat, char *block)
//{
//	zfill(block);
//	return EXIT_SUCCESS;
//}

int writeContent(ArchiveFile *archiveFile, const char *path, blkcnt_t n_blocks)
{
	if (!_strcmp(archiveFile->path, path)) {
		return error(FILE_IS_ARCHIVE_ERR, path);
	}
	int fd = open(path, O_RDONLY);
	if (fd == SYSCALL_ERR_CODE) {
		return error(CANT_OPEN_FILE_ERR, path);
	}
	for (blkcnt_t i = 0; i < n_blocks; i++) {
		if (i == n_blocks - 1) zfill(archiveFile->block);
		if (read(fd, archiveFile->block, BLOCKSIZE) == SYSCALL_ERR_CODE) {
			return error(CANT_READ_ERR, path);
		}
		if (write(archiveFile->fd, archiveFile->block, BLOCKSIZE) == SYSCALL_ERR_CODE) {
			return error(CANT_WRITE_ERR, archiveFile->path);
		}
	}
	close(fd);
	return EXIT_SUCCESS;
}

int appendEnd(ArchiveFile *archiveFile)
{
	for (int i = 0; i < END_OF_ARCHIVE_N_BLOCKS; i++) {
		zfill(archiveFile->block);
		write(archiveFile->fd, archiveFile->block, BLOCKSIZE);
	}
	return EXIT_SUCCESS;
}

void zfill(char *block)
{
	for (int i = 0; i < BLOCKSIZE; i++) {
		block[i] = 0;
	}
}

inline blkcnt_t getNumBlocks(off_t n_bytes)
{
	return (n_bytes - 1) / BLOCKSIZE + 1;
}

int error(const char *message, const char *messageArg)
{
	_dprintf(STDERR_FILENO, message, messageArg);
	return EXIT_FAILURE;
}
