#include "modes.h"
#include "tar_header.h"
#include "utils/_string.h"
#include "file/archived_file.h"
#include "file/archive.h"
#include "error/error.h"
#include <unistd.h>
#include <stdlib.h>

#define END_OF_ARCHIVE_SIZE 2 * BLOCKSIZE

static int append(const char *path, Archive *archive);
static int writeHeader(const ArchivedFile *file, Archive *archive);
static int writeContent(const ArchivedFile *file, Archive *archive);
static int appendEnd(Archive *archive);
static PosixHeader getZeroFilledPosixHeader();
static int cleanupAfterFailure(Params *params);

int c_mode(Params *params)
{
	Archive archive;
	if (initArchive(&archive, params->archivePath)) {
		return cleanupAfterFailure(params);
	}
	while (params->filePaths) {
		if (append(params->filePaths->path, &archive)) return cleanupAfterFailure(params);
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
	ArchivedFile file;
	if (initArchivedFile(&file, path)) {
		return EXIT_FAILURE;
	}
	if (writeHeader(&file, archive)) {
		return EXIT_FAILURE;
	}
	if (writeContent(&file, archive)) {
		return EXIT_FAILURE;
	}
	closeArchivedFile(&file);
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
	if (readFile(file) == SYSCALL_ERR_CODE) {
		closeArchivedFile(file);
		return error(CANT_READ_ERR, file->path);
	}
	if (writeToArchive(file, archive) == SYSCALL_ERR_CODE) {
		closeArchivedFile(file);
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

int cleanupAfterFailure(Params *params)
{
	while (params->filePaths) {
		PathNode *current = params->filePaths;
		params->filePaths = params->filePaths->next;
		free(current);
	}
	return EXIT_FAILURE;
}
