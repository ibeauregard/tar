#include "modes.h"
#include "tar_header.h"
#include "utils/_string.h"
#include "utils/_stdio.h"
#include "constants.h"
#include "file/archived_file.h"
#include "file/archive.h"
#include "error/error.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#define END_OF_ARCHIVE_SIZE 2 * BLOCKSIZE

typedef struct stat Stat;

static int append(const char *path, Archive *archive);
//static int write_header(Archive *archive);//, const Stat *fileStat);
static int writeContent(const char *path, off_t st_size, Archive *archive);
static int appendEnd(Archive *archive);

int c_mode(Params *params)
{
	Archive archive;
	if (initArchive(&archive, params->archivePath)) {
		return EXIT_FAILURE;
	}
	while (params->filePaths) {
		if (append(params->filePaths->path, &archive)) return EXIT_FAILURE;
		PathNode *current = params->filePaths;
		params->filePaths = params->filePaths->next;
		free(current);
	}
	if (appendEnd(&archive)) {
		return EXIT_FAILURE;
	}
	closeArchive(&archive);
	return EXIT_SUCCESS;
}

int append(const char *path, Archive *archive)
{
	if (!_strcmp(archive->path, path)) {
		return error(FILE_IS_ARCHIVE_ERR, path);
	}
	Stat fileStat;
	if (lstat(path, &fileStat) == SYSCALL_ERR_CODE) {
		return error(STAT_ERR, path);
	}
//	write_header(archive);//, &fileStat);
	if (writeContent(path, fileStat.st_size, archive)) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

//int write_header(Archive *archiveFile)//, const Stat *fileStat)
//{
//	PosixHeader header = {};
//	write(archiveFile->fd, &header, BLOCKSIZE);
//	zfill(archiveFile->block);
//	return EXIT_SUCCESS;
//}

int writeContent(const char *path, off_t st_size, Archive *archive)
{
	ArchivedFile file;
	if (initArchivedFile(&file, path, st_size)) {
		return EXIT_FAILURE;
	}
	if (readFile(&file) == SYSCALL_ERR_CODE) {
		closeArchivedFile(&file);
		return error(CANT_READ_ERR, file.path);
	}
	if (writeToArchive(&file, archive) == SYSCALL_ERR_CODE) {
		closeArchivedFile(&file);
		return error(CANT_WRITE_ERR, archive->path);
	}
	closeArchivedFile(&file);
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
