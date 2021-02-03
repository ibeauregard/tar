#include "modes.h"
#include "tar_header.h"
#include "utils/_string.h"
#include "file/archived_file.h"
#include "file/archive.h"
#include "error/error.h"
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>

typedef struct dirent Dirent;

static int handlePath(char *path, Archive *archive);
static int writeEntry(const ArchivedFile *file, Archive *archive);
static int writeHeader(const ArchivedFile *file, Archive *archive);
static int writeContent(const ArchivedFile *file, Archive *archive);
static int appendDirContent(const ArchivedFile *dir, Archive *archive);
static char* build_path(char* fullPath, const char* dirPath, const char* name);
static PosixHeader getZeroFilledPosixHeader();

int c_mode(Params *params)
{
	Archive archive;
	if (initArchive(&archive, params->archivePath)) {
		return EXIT_FAILURE;
	}
	while (params->filePaths) {
		if (handlePath(params->filePaths->path, &archive)) {
			finalizeArchive(&archive);
			return EXIT_FAILURE;;
		}
		PathNode *current = params->filePaths;
		params->filePaths = params->filePaths->next;
		free(current);
	}
	int status = appendEnd(&archive);
	finalizeArchive(&archive);
	return status;
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
	int status = writeEntry(&file, archive);
	finalizeArchivedFile(&file);
	return status;
}

int writeEntry(const ArchivedFile *file, Archive *archive)
{
	if (writeHeader(file, archive)) {
		return EXIT_FAILURE;
	}
	if (writeContent(file, archive)) {
		return EXIT_FAILURE;
	}
	if (file->type == DIRTYPE) {
		return appendDirContent(file, archive);
	}
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

int writeContent(const ArchivedFile *file, Archive *archive)
{
	return readFile(file) || writeToArchive(file, archive);
}

int appendDirContent(const ArchivedFile *dir, Archive *archive)
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
		if (handlePath(build_path(fullpath, dir->path, entry->d_name), archive)) {
			return EXIT_FAILURE;
		}
	}
	closedir(folder);
	return EXIT_SUCCESS;
}

char* build_path(char* fullPath, const char* dirPath, const char* name)
{
	return _strcat(_strcpy(fullPath, dirPath), name);
}

PosixHeader getZeroFilledPosixHeader()
{
	static PosixHeader header;
	return header;
}
