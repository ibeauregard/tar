#include "modes.h"
#include "tar_header.h"
#include "utils/_string.h"
#include "file/archived_file.h"
#include "file/archive.h"
#include "error/error.h"
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>

#define END_OF_ARCHIVE_SIZE 2 * BLOCKSIZE

typedef struct dirent Dirent;

static int handlePath(char *path, Archive *archive);
static int writeHeader(const ArchivedFile *file, Archive *archive);
static int appendDirectory(const ArchivedFile *dir, Archive *archive);
static char* build_path(char* fullPath, const char* dirPath, const char* name);
static int writeContent(const ArchivedFile *file, Archive *archive);
static int appendEnd(Archive *archive);
static PosixHeader getZeroFilledPosixHeader();

int c_mode(Params *params)
{
	Archive archive;
	if (initArchive(&archive, params->archivePath)) {
		return EXIT_FAILURE;
	}
	while (params->filePaths) {
		if (handlePath(params->filePaths->path, &archive)) {
			return EXIT_FAILURE;;
		}
		PathNode *current = params->filePaths;
		params->filePaths = params->filePaths->next;
		free(current);
	}
	if (appendEnd(&archive)) {
		return EXIT_FAILURE;;
	}
	destructArchive(&archive);
	return EXIT_SUCCESS;
}

int handlePath(char *path, Archive *archive)
{
	if (!_strcmp(archive->path, path)) {
		return error(FILE_IS_ARCHIVE_ERR, path);
	}
	ArchivedFile file;
	if (initArchivedFile(&file, path)) {
		return EXIT_FAILURE;
	}
	if (writeHeader(&file, archive)) {
		destructArchivedFile(&file);
		return EXIT_FAILURE;
	}
	if (file.type == DIRTYPE) {
		int result = appendDirectory(&file, archive);
		destructArchivedFile(&file);
		return result;
	}
	if (writeContent(&file, archive)) {
		destructArchivedFile(&file);
		return EXIT_FAILURE;
	}
	destructArchivedFile(&file);
	return EXIT_SUCCESS;
}

int writeHeader(const ArchivedFile *file, Archive *archive)
{
	PosixHeader header = getZeroFilledPosixHeader();
	fillHeader(file, &header);
	if (write(archive->fd, &header, BLOCKSIZE) == SYSCALL_ERR_CODE) {
		return error(CANT_WRITE_ERR, archive->path);
	}
	return EXIT_SUCCESS;
}

int appendDirectory(const ArchivedFile *dir, Archive *archive)
{
	DIR *folder = opendir(dir->path);
	Dirent *entry;
	while ((entry = readdir(folder))) {
		if (!_strcmp(entry->d_name, ".") || !_strcmp(entry->d_name, "..")) {
			continue;
		}
		char fullpath[_strlen(dir->path)
						+ _strlen(entry->d_name)
						+ 1];
		handlePath(build_path(fullpath, dir->path, entry->d_name), archive);
	}
	closedir(folder);
	return EXIT_SUCCESS;
}

char* build_path(char* fullPath, const char* dirPath, const char* name)
{
	return _strcat(_strcpy(fullPath, dirPath), name);
}

int writeContent(const ArchivedFile *file, Archive *archive)
{
	if (readFile(file) == SYSCALL_ERR_CODE) {
		return error(CANT_READ_ERR, file->path);
	}
	if (writeToArchive(file, archive) == SYSCALL_ERR_CODE) {
		return error(CANT_WRITE_ERR, archive->path);
	}
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

PosixHeader getZeroFilledPosixHeader()
{
	static PosixHeader header;
	return header;
}
