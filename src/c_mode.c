#include "modes.h"
#include "tar_header.h"
#include "utils/_string.h"
#include "file/archived_file.h"
#include "file/archive.h"
#include "error/error.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define END_OF_ARCHIVE_SIZE 2 * BLOCKSIZE

static int append(const char *path, Archive *archive);
static int write_header(const ArchivedFile *file, Archive *archive);
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
	ArchivedFile file;
	if (initArchivedFile(&file, path)) {
		return EXIT_FAILURE;
	}
	if (write_header(&file, archive)) {
		return EXIT_FAILURE;
	}
	if (writeContent(&file, archive)) {
		return EXIT_FAILURE;
	}
	closeArchivedFile(&file);
	return EXIT_SUCCESS;
}

int write_header(const ArchivedFile *file, Archive *archive)
{
	PosixHeader header = getZeroFilledPosixHeader();
	fillHeader(file, &header);

	write(archive->fd, &header, BLOCKSIZE);
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
