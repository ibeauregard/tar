#include <stdlib.h>
#include <fcntl.h>            // For open
#include <unistd.h>           // For read

#include "tar_header.h"
#include "utils/_string.h"    // For _strlen
#include "modes.h"

typedef struct s_ParsedTar {
	PosixHeader *header;
	char *contents;
	parsedTar *next;
} ParsedTar;

int x_mode(Params *params)
{
	ParsedTar *parsedTar = parseTar(Params->archivePath);
	// If no -f arguments, tar will extract the whole tar
	// otherwise will extract only the specified files
	if (!params->filePaths) {
		extractAllFiles(parsedTar);
	} else {
		extractSomeFiles(parsedTar, params);
	}
	return EXIT_SUCCESS;
}

static ParsedTar *parseTar(char *archivePath)
{
	int archivefd = open(archivePath, O_RDONLY);
	char currentBlock[BLOCKSIZE];
	int consecutiveEmptyBlocks = 0;
	do {
	// TODO: NEXT STEP FINISH SKELETON
		read(archivefd, (void) currentBlock, BLOCKSIZE);
		if (checkEmpty(currentBlock)) {
			consecutiveEmptyBlocks++;
			continue;
		} else {
			consecutiveEmptyBlocks = 0;
		}
	} while (consecutiveEmptyBlocks < 2);
}

/* Function: Extract all files in tar archive
 */
static int extractAllFiles(ParsedTar *parsedTar) {
	while (parsedTar) {
		extractFileOrDir(parsedTar);
		parsedTar = parsedTar->next;
	}
}

/* Function: Extract only the specified files in *params
 * -----------------------------------------------------
 * If extracting a nested file, must be in the form "dirName/fileName".
 * If a directory or nested directory is specified e.g. "dirName/dirName",
 * all nested directories and files will be extracted too.
 */
static int extractSomeFiles(ParsedTar *parsedTar, Params *params) {
	PathNode *pathNode = params->filePaths;
	while (pathNode) {
		PathNode *current = pathNode;
		findAndExtract(parsedTar, current);
		pathNode = pathNode->next;
		free(current);
	}
	return EXIT_SUCCESS;
}

static int findThenExtract(ParsedTar *parsedTar, PathNode *current) {
	while (parsedTar) {
		char *parsedName = parsedTar->header->name;
		char *pathName = current->path;
		if (!_strncmp(parsedName, pathName, _strlen(pathName))
			extractFileOrDir(parsedTar);
		parsedTar = parsedTar->next;
	}
}

static int extractFileOrDir(ParsedTar *parsedTar) {
	createFile()
	changePermissions()
	writeFile()
}
