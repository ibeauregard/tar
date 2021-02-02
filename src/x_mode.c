#include <stdio.h>            // For debugging
#include <stdlib.h>
#include <fcntl.h>            // For open
#include <unistd.h>           // For read, STDERR_FILENO
// #include <errno.h>            // For strerror
#include <string.h>           // For strerror

#include "tar_header.h"
#include "utils/_string.h"    // For _strlen
#include "modes.h"

typedef struct s_ParsedTar {
	PosixHeader *header;
	char *contents;
	struct s_ParsedTar *next;
} ParsedTar;

static ParsedTar *parseTar(char *archivePath);
static ParsedTar *newParsedTar();
static int checkEndOfArchive(int archivefd);
static int addNode(ParsedTar *headNode, ParsedTar *lastNode);
static int parseHeader(int archivefd, PosixHeader *header);
static int parseContents(int archivefd, ParsedTar *nextNode);

int x_mode(Params *params)
{
	ParsedTar *parsedTar = parseTar(params->archivePath);
	(void) parsedTar;
	// If no -f arguments, tar will extract the whole tar
	// otherwise will extract only the specified files
	/*
	if (!params->filePaths) {
		extractAllFiles(parsedTar);
	} else {
		extractSomeFiles(parsedTar, params);

	}
	freeParsedTar(parsedTar);
	*/
	return EXIT_SUCCESS;
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
	int result;
	do {
		if ((result = checkEndOfArchive(archivefd)))
			break;
		if (addNode(headNode, lastNode) == -1)
			break;
		parseHeader(archivefd, lastNode->header);
		parseContents(archivefd, lastNode);
	} while (!endOfArchive);
	printf("result: %d\n", result);
	return headNode;
}

/* Function: Checks if next BLOCKSIZE * 2 bytes are null
 * -----------------------------------------------------
 * .tar archives terminate their archives with 2 blocks of null bytes.
 * After checking, use lseek to return fildes pointer to where it was 
 * before being checked so that parseHeader and parseContent work.
 */ 
static ParsedTar *newParsedTar() 
{
	ParsedTar *newParsedTar = malloc(sizeof(ParsedTar));
	if (!newParsedTar)
		return NULL;
	newParsedTar->next = NULL;
	return newParsedTar;
}

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

static int addNode(ParsedTar *headNode, ParsedTar *lastNode)
{
	ParsedTar *nextNode = newParsedTar();
	if (!nextNode)
		return -1;
	if (headNode == NULL) {
		headNode = lastNode = nextNode;
	} else {
		lastNode->next = nextNode;
		lastNode = lastNode->next;
	}
	return 0;
}

static int parseHeader(int archivefd, PosixHeader *header) 
{
	char nextBlock[BLOCKSIZE + 1] = { '\0' };
	int bytesRead = read(archivefd, nextBlock, BLOCKSIZE);
	// First check that we can read BLOCKSIZE bytes from fildes
	if (bytesRead < BLOCKSIZE) {
		dprintf(STDERR_FILENO, "Error: Cannot read BLOCKSIZE bytes\n");
		lseek(archivefd, -bytesRead, SEEK_CUR);
		return -1;
	}
	// If we can read BLOCKSIZE bytes, restart from beginning and parse
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

static int _isdigit(char d) {
	if (d < '0' || d > '9')
		return 0;
	return 1;
}

static int getValueOfDigit(char d, int base) 
{
	if (base > 35 || base < 1)
		return -1;
	else {
		if (_isdigit(d))
			return d - '0';
		if ((d >= 'a' && d <= 'a' + base - 10))
			return d - 'a';
		if ((d >= 'A' && d <= 'Z' + base - 10))
			return d - 'A';
		return -1;
	}
		
}

static long _strtol(const char *restrict str, char **restrict endptr, int base)
{
	if (base > 35)
		return 0;
	int result = 0;
	int sign = 1;
	if (*str == '-') {
		sign = -1;
		str++;
	}
	for (int i = 0; *(str + i); i++) {
		char d = *(str + i);
		int n;
		if ((n = getValueOfDigit(d, base)) == -1) {
			if (endptr != NULL)
				*endptr = (char *) str + i;
			return result * sign;
		}
		result *= base;
		result += n;
	}
	return result * sign;
}

static int parseContents(int archivefd, ParsedTar *nextNode) 
{
	(void) archivefd;
	long size = _strtol(nextNode->header->size, NULL, 8);
	printf("%ld\n", size);
	return 0;
}

// /* Function: Extract all files in tar archive
//  * ------------------------------------------
//  */
// static int extractAllFiles(ParsedTar *parsedTar) {
// 	while (parsedTar) {
// 		extractFileOrDir(parsedTar);
// 		parsedTar = parsedTar->next;
// 	}
// }
// 
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
// 			extractFileOrDir(parsedTar);
// 		parsedTar = parsedTar->next;
// 	}
// }
// 
// static int extractFileOrDir(ParsedTar *parsedTar) {
// 	createFile()
// 	changePermissions()
// 	writeFile()
// }
