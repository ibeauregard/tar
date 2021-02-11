#include "tar_list.h"
#include "archive.h"
#include "error.h"
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

int dumpToArchive(TarList *list, const Params *params, bool append)
{
	Archive archive;
	if (initArchive(&archive, params->archivePath, append)) {
		return finalizeTarList(list);
	}
	TarNode *node = list->first;
	while(node) {
		if (dumpHeader(node->headerData, &archive)
			|| dumpContent(node->headerData, &archive)) {
			return finalizeTarList(list);
		}
		finalizeHeaderData(node->headerData);
		TarNode *current = node;
		node = node->next;
		free(current);
	}
	int status = appendEnd(&archive);
	finalizeArchive(&archive);
	return status;
}

int dumpHeader(const HeaderData *headerData, const Archive *archive)
{
	PosixHeader header = getZeroFilledPosixHeader();
	if (write(archive->fd, getHeaderFromData(headerData, &header), BLOCKSIZE) == SYSCALL_ERR_CODE) {
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
	if (read(fd, buffer, numBytes) == SYSCALL_ERR_CODE) {
		return error(CANT_READ_ERR, headerData->name);
	}
	if (write(archive->fd, buffer, numBytes) == SYSCALL_ERR_CODE) {
		return error(CANT_WRITE_ERR, archive->path);
	}
	return EXIT_SUCCESS;
}

void zfillLastBlock(char *buffer, size_t numBlocks)
{
	size_t totalBytes = numBlocks * BLOCKSIZE;
	for (size_t i = (numBlocks - 1) * BLOCKSIZE; i < totalBytes; i++) {
		buffer[i] = 0;
	}
}

int finalizeTarList(TarList *list)
{
	TarNode *node = list->first;
	while (node) {
		finalizeHeaderData(node->headerData);
		TarNode *current = node;
		node = node->next;
		free(current);
	}
	return EXIT_FAILURE;
}
