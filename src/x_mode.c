#include <stdio.h>            // For debugging
#include <stdlib.h>
#include <fcntl.h>            // For open
#include <unistd.h>           // For read, STDERR_FILENO
// #include <errno.h>            // For strerror?
// #include <string.h>           // For strerror

#include "tar_header.h"
#include "utils/_string.h"    // For _strlen
#include "utils/_stdlib.h"    // For _strtol
#include "modes.h"

typedef struct s_ParsedTar {
	PosixHeader *header;
	char *contents;
	struct s_ParsedTar *next;
} ParsedTar;

static ParsedTar *parseTar(char *archivePath);
static ParsedTar *newParsedTar();
static int checkEndOfArchive(int archivefd);
static int addNode(ParsedTar **headNode, ParsedTar **lastNode);
static int parseHeader(int archivefd, ParsedTar *lastNode);
static int parseContents(int archivefd, ParsedTar *nextNode);
static void printParsedTar(ParsedTar *parsedTar);

int x_mode(Params *params)
{
	ParsedTar *parsedTar = parseTar(params->archivePath);
	(void) parsedTar;
	printParsedTar(parsedTar);
	// If no -f arguments, tar will extract the whole tar
	// otherwise will extract only the specified files
	/*
	if (!params->filePaths) {
		extractAllFiles(parsedTar);
	} 
	else {
		extractSomeFiles(parsedTar, params);

	}
	freeParsedTar(parsedTar);
	*/
	return EXIT_SUCCESS;
}

/* Function: Prints out contents of ParsedTar for debugging
 * -------------------------------------------------------
 */
static void printParsedTar(ParsedTar *parsedTar)
{
	while (parsedTar) {
		printf("name: %s\n", parsedTar->header->name);
		write(1, parsedTar->contents, BLOCKSIZE);
		printf("\n");
		parsedTar = parsedTar->next;
	}
}

/* Function: Parses .tar archive into struct ParsedTar for individual files
 * ------------------------------------------------------------------------
 * parsedTar() parses all the contents of a tar archive and holds it in a  
 * ParsedTar structure. Each ParsedTar struct holds the header and contents 
 * of a single file along with a pointer to the next ParsedTar.
 */ 
static ParsedTar *parseTar(char *archivePath)
{
	int archivefd = open(archivePath, O_RDONLY);
	int endOfArchive = 0;
	ParsedTar *headNode = NULL; 
	ParsedTar *lastNode = headNode;
	do {
		if (checkEndOfArchive(archivefd))
			break;
		if (addNode(&headNode, &lastNode) == -1)
			break;
		parseHeader(archivefd, lastNode);
		parseContents(archivefd, lastNode);
	} while (!endOfArchive);
	return headNode;
}

static ParsedTar *newParsedTar() 
{
	ParsedTar *newParsedTar = malloc(sizeof(ParsedTar));
	if (!newParsedTar)
		return NULL;
	newParsedTar->header = NULL;
	newParsedTar->contents = NULL;
	newParsedTar->next = NULL;
	return newParsedTar;
}

/* Function: Checks if next BLOCKSIZE * 2 bytes are null
 * -----------------------------------------------------
 * .tar archives terminate their archives with 2 blocks of null bytes.
 * After checking, use lseek to return fildes pointer to where it was 
 * before being checked so that parseHeader and parseContent work.
 */ 
static int checkEndOfArchive(int archivefd) 
{
	char nextTwoBlocks[BLOCKSIZE * 2 + 1] = { '\0' };
	int bytesRead = read(archivefd, nextTwoBlocks, BLOCKSIZE * 2);
	if (bytesRead < BLOCKSIZE * 2) {
		dprintf(STDERR_FILENO, "Error: Cannot read 2x BLOCKSIZE bytes\n");
		lseek(archivefd, -bytesRead, SEEK_CUR);
		return -1;
	}
	for (int i = 0; i < BLOCKSIZE * 2; i++) {
		if (nextTwoBlocks[i] != '\0') {
			lseek(archivefd, -bytesRead, SEEK_CUR);
			return 0;
		}
	}
	return 1;
}

/* Function: Adds new ParsedTar node to the existing linked list
 * -------------------------------------------------------------
 * @**headNode: Address of the pointer to the first node
 * @**lastNode: Address to the pointer to the last node
 * If there are no nodes, point headNode and lastNode to new node.
 * Otherwise, do nothing with headNode and link new node to lastNode.
 */ 
static int addNode(ParsedTar **headNode, ParsedTar **lastNode)
{
	ParsedTar *nextNode = newParsedTar();
	if (!nextNode)
		return -1;
	if (*headNode == NULL) {
		*headNode = *lastNode = nextNode;
	} else {
		(*lastNode)->next = nextNode;
		*lastNode = (*lastNode)->next;
	}
	return 0;
}

/* Function: Parses header in .tar file and puts it in PosixHeader struct
 * ----------------------------------------------------------------------
 */
static int parseHeader(int archivefd, ParsedTar *lastNode) 
{
	PosixHeader *header = malloc(sizeof(PosixHeader));
	lastNode->header = header;
	char nextBlock[BLOCKSIZE + 1] = { '\0' };
	int bytesRead = read(archivefd, nextBlock, BLOCKSIZE);
	if (bytesRead < BLOCKSIZE) {
		dprintf(STDERR_FILENO, "Error: Cannot read BLOCKSIZE bytes\n");
		lseek(archivefd, -bytesRead, SEEK_CUR);
		return -1;
	}
	lseek(archivefd, -bytesRead, SEEK_CUR);
	bytesRead = 0;
	bytesRead += read(archivefd, header->name, 100);
	bytesRead += read(archivefd, header->mode, 8);
	bytesRead += read(archivefd, header->uid, 8);
	bytesRead += read(archivefd, header->gid, 8);
	bytesRead += read(archivefd, header->size, 12);
	bytesRead += read(archivefd, header->mtime, 12);
	bytesRead += read(archivefd, header->chksum, 8);
	bytesRead += read(archivefd, &header->typeflag, 1);
	bytesRead += read(archivefd, header->linkname, 100);
	bytesRead += read(archivefd, header->magic, 6);
	bytesRead += read(archivefd, header->version, 2);
	bytesRead += read(archivefd, header->uname, 32);
	bytesRead += read(archivefd, header->gname, 32);
	bytesRead += read(archivefd, header->devmajor, 8);
	bytesRead += read(archivefd, header->devminor, 8);
	bytesRead += read(archivefd, header->prefix, 155);
	lseek(archivefd, BLOCKSIZE - bytesRead, SEEK_CUR);
	return 0;
} 
static int parseContents(int archivefd, ParsedTar *nextNode) 
{
	long size = _strtol(nextNode->header->size, NULL, 8);
	if (size == 0)
		return 0;
	long blockCount = size / (BLOCKSIZE + 1) + 1;
	char *contents = calloc(BLOCKSIZE * (int) blockCount, sizeof(char));
	nextNode->contents = contents;
	int bytesRead = read(archivefd, nextNode->contents, BLOCKSIZE * blockCount);
	// printf("name: %s\n", nextNode->header->name);
	// write(1, nextNode->contents, BLOCKSIZE * blockCount);
	// First check that we can read BLOCKSIZE bytes from fildes
	if (bytesRead < BLOCKSIZE * blockCount) {
		dprintf(STDERR_FILENO, "Error: Cannot read contents\n");
		lseek(archivefd, -bytesRead, SEEK_CUR);
		return -1;
	}
	return 0;
}

/* Function: Extract all files in tar archive
 * ------------------------------------------
 */
// static int extractAllFiles(ParsedTar *parsedTar) {
// 	while (parsedTar) {
// 		getFileType(parsedTar);
// 		parsedTar = parsedTar->next;
// 	}
// }
// 
// static int getFileType(parsedTar) 
// {
// 	char *mode = parsedTar->header->mode;
// 
// }

// /* Function: Extract only the specified files in *params
//  * -----------------------------------------------------
//  * If extracting a nested file, must be in the form "dirName/fileName".
//  * If a directory or nested directory is specified e.g. "dirName/dirName",
//  * all nested directories and files will be extracted too.
//  */
// static int extractSomeFiles(ParsedTar *parsedTar, Params *params) {
// 	PathNode *pathNode = params->filePaths;
// 	while (pathNode) {
// 		PathNode *current = pathNode;
// 		findAndExtract(parsedTar, current);
// 		pathNode = pathNode->next;
// 		free(current);
// 	}
// 	return EXIT_SUCCESS;
// }
// 
// static int findThenExtract(ParsedTar *parsedTar, PathNode *current) {
// 	while (parsedTar) {
// 		char *parsedName = parsedTar->header->name;
// 		char *pathName = current->path;
// 		if (!_strncmp(parsedName, pathName, _strlen(pathName))
// 			extractFile(parsedTar);
// 		parsedTar = parsedTar->next;
// 	}
// }



