#include "tar_list.h"
#include "file/archive.h"
#include "error/error.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

static int dumpHeader(const HeaderData *headerData, const Archive *archive);
static int dumpContent(const HeaderData *headerData, const Archive *archive);
static void zfillLastBlock(char *buffer, size_t numBlocks);

TarList getNewTarList()
{
	static TarList list;
	return list;
}

int dumpToArchive(TarList *list, const char *archivePath)
{
	Archive archive;
	if (initArchive(&archive, archivePath)) {
		return finalizeTarList(list);
	}
	while(list->node) {
		if (dumpHeader(list->node->headerData, &archive)
			|| dumpContent(list->node->headerData, &archive)) {
			return finalizeTarList(list);
		}
		finalizeHeaderData(list->node->headerData);
		TarNode *current = list->node;
		list->node = list->node->next;
		free(current);
	}
	int status = appendEnd(&archive);
	finalizeArchive(&archive);
	return status;
}

int dumpHeader(const HeaderData *headerData, const Archive *archive)
{
	PosixHeader header = getZeroFilledPosixHeader();
	if (write(archive->fd, getFilledHeader(headerData, &header), BLOCKSIZE) == SYSCALL_ERR_CODE) {
		return error(CANT_WRITE_ERR, archive->path);
	}
	return EXIT_SUCCESS;
}

int dumpContent(const HeaderData *headerData, const Archive *archive)
{
	size_t numBlocks = getNumBlocks(headerData);
	if (numBlocks == 0) return EXIT_SUCCESS;
	const size_t numBytes = numBlocks * BLOCKSIZE;
	char buffer[numBytes];
	int fd = open(headerData->name, O_RDONLY);
	if (fd == SYSCALL_ERR_CODE) {
		return error(CANT_OPEN_FILE_ERR, headerData->name);
	}
	zfillLastBlock(buffer, numBlocks);
	if (read(fd, buffer, numBytes)
		== SYSCALL_ERR_CODE) {
		return error(CANT_READ_ERR, headerData->name);
	}
	if (write(archive->fd, buffer, numBytes)
		== SYSCALL_ERR_CODE) {
		return error(CANT_WRITE_ERR, archive->path);
	}
	return EXIT_SUCCESS;
}

int finalizeTarList(TarList *list)
{
	while (list->node) {
		finalizeHeaderData(list->node->headerData);
		TarNode *current = list->node;
		list->node = list->node->next;
		free(current);
	}
	return EXIT_FAILURE;
}

void zfillLastBlock(char *buffer, size_t numBlocks)
{
	size_t totalBytes = numBlocks * BLOCKSIZE;
	for (size_t i = (numBlocks - 1) * BLOCKSIZE; i < totalBytes; i++) {
		buffer[i] = 0;
	}
}
