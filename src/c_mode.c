#include "modes.h"
#include "tar_header.h"
#include "utils/_string.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#define ARCHIVE_FLAGS O_CREAT|O_WRONLY|O_TRUNC
#define ARCHIVE_MODE S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH

typedef struct stat Stat;

static int append(int archiveFD, const char *path, char *block);
//static int write_header(int archiveFD, Stat *fileStat, char *block);
static int writeContent(int archiveFD, const char *path, char *block, blkcnt_t n_blocks);
static void zfill(char *block);
static blkcnt_t getNumBlocks(off_t n_bytes);

int c_mode(Params *params)
{
	// TODO check for error
	int archiveFD = open(params->archivePath, ARCHIVE_FLAGS, ARCHIVE_MODE);
	char block[BLOCKSIZE];
	PathNode *pathNode = params->filePaths;
	while (pathNode) {
		if (append(archiveFD, pathNode->path, block)) return EXIT_FAILURE;
		PathNode *current = pathNode;
		pathNode = pathNode->next;
		free(current);
	}
	close(archiveFD);
	return EXIT_SUCCESS;
}

int append(int archiveFD, const char *path, char *block)
{
	Stat fileStat;
	// TODO check for error
	stat(path, &fileStat);
//	write_header(archiveFD, &fileStat, block);
	writeContent(archiveFD, path, block, getNumBlocks(fileStat.st_size));
	return EXIT_SUCCESS;
}

//int write_header(int archiveFD, Stat *fileStat, char *block)
//{
//	zfill(block);
//	return EXIT_SUCCESS;
//}

int writeContent(int archiveFD, const char *path, char *block, blkcnt_t n_blocks)
{
	// TODO check for error
	int fd = open(path, O_RDONLY);
	for (blkcnt_t i = 0; i < n_blocks; i++) {
		if (i == n_blocks - 1) zfill(block);
		// TODO check for error
		read(fd, block, BLOCKSIZE);
		// TODO check for error
		write(archiveFD, block, BLOCKSIZE);
	}
	close(fd);
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
