#include <stdio.h>            // For debugging
#include <stdlib.h>
#include <fcntl.h>            // For open
#include <unistd.h>           // For read, STDERR_FILENO
// #include <errno.h>            // For strerror?
// #include <string.h>           // For strerror

#include "tar_header.h"
#include "header_data.h"
#include "utils/_string.h"    // For _strlen
#include "utils/_stdlib.h"    // For _strtol
#include "modes.h"
#include "tar_node.h"

// Functions for parsing tar archive
static TarNode *parseTar(char *archivePath);
static TarNode *newParsedTar();
static int checkEndOfArchive(int archivefd);
static int addNode(TarNode **headNode, TarNode **lastNode);
static int parseHeader(int archivefd, TarNode *lastNode);
static int skipContents(int archivefd, TarNode *lastNode);
static void printTarNode(TarNode *parsedTar);

// Functions for creating files
static int extractFiles(Params *params, TarNode *parsedTar);
static void createFile(int archivefd, TarNode *parsedTar);
static int createREGTYPE(int archivefd, TarNode *tarNode);
static int createLNKTYPE(int archivefd, TarNode *tarNode);
static int createSYMTYPE(int archivefd, TarNode *tarNode);
static int createDIRTYPE(int archivefd, TarNode *tarNode);

int x_mode(Params *params)
{
	TarNode *parsedTar = parseTar(params->archivePath);
	(void) parsedTar;
	printTarNode(parsedTar);
	extractFiles(params, parsedTar);
	/*
	freeParsedTar(parsedTar);
	*/
	return EXIT_SUCCESS;
}

/* Function: Prints out contents of TarNode for debugging
 * -------------------------------------------------------
 */
static void printTarNode(TarNode *parsedTar)
{
	while (parsedTar) {
		printf("name: %s\n", parsedTar->header->name);
		// write(1, parsedTar->contents, BLOCKSIZE);
		printf("\n");
		parsedTar = parsedTar->next;
	}
}

/* Function: Parses .tar archive into struct TarNode for individual files
 * ------------------------------------------------------------------------
 * parsedTar() parses all the headers of a tar archive and holds it in a  
 * TarNode structure. Each TarNode struct holds the header of a single file
 * along with a pointer to the next TarNode. Returns head of linked list.
 */ 
static TarNode *parseTar(char *archivePath)
{
	int archivefd = open(archivePath, O_RDONLY);
	TarNode *headNode = NULL;
	TarNode *lastNode = headNode;
	do {
		if (checkEndOfArchive(archivefd))
			break;
		if (addNode(&headNode, &lastNode) == -1)
			return 1;
		if (parseHeader(archivefd, lastNode) == -1);
			return 1;
		skipContents(archivefd, lastNode);
	} while (true);
	return headNode;
}

static TarNode *newParsedTar()
{
	TarNode *newParsedTar = malloc(sizeof(TarNode));
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

/* Function: Adds new TarNode node to the existing linked list
 * -------------------------------------------------------------
 * @**headNode: Address of the pointer to the first node
 * @**lastNode: Address to the pointer to the last node
 * If there are no nodes, point headNode and lastNode to new node.
 * Otherwise, do nothing with headNode and link new node to lastNode.
 */ 
static int addNode(TarNode **headNode, TarNode **lastNode)
{
	TarNode *nextNode = newParsedTar();
	if (!nextNode) {
		dprintf(STDERR_FILENO, "Error: Could not allocate memory\n");
		return -1;
	}
	if (*headNode == NULL) {
		*headNode = *lastNode = nextNode;
	} else {
		(*lastNode)->next = nextNode;
		*lastNode = (*lastNode)->next;
	}
	return 0;
}

static int getContentsSize(TarNode *tarNode)
{
	long size = _strtol(tarNode->header->size, NULL, 8);
	if (size == 0)
		return 0;
	int contentBlocks = size / (BLOCKSIZE + 1) + 1;
	int contentSize = contentBlocks * BLOCKSIZE;
	return contentSize;
}


/* Function: Converts PosixHeader to our custom HeaderData
 * -------------------------------------------------------
 * PosixHeader is the information as stored in the .tar archive, but it is
 * not practical for using across the program since all data is in ASCII.
 * The HeaderData structure uses appropriate data types for ease of manipulation.
static int convertHeader(PosixHeader* posixHeader)
{
	HeaderData *headerData = malloc(sizeof(HeaderData));
	_strcpy(headerData->name, posixHeader->name);
	headerData->permissions = _strtol(posixHeader->mode, NULL, 8)
}
 */

/* Function: Parses header in .tar file and puts it in PosixHeader struct
 * ----------------------------------------------------------------------
 */
static int parseHeader(int archivefd, TarNode *lastNode)
{
	PosixHeader *header = malloc(sizeof(PosixHeader));
	lastNode->header = header;
	int bytesRead = read(archivefd, header, BLOCKSIZE);
	if (bytesRead < BLOCKSIZE) {
		dprintf(STDERR_FILENO, "Error: Cannot read BLOCKSIZE bytes\n");
		lseek(archivefd, -bytesRead, SEEK_CUR);
		return -1;
	}
	if (_strtol(header->chksum, NULL, 8) != computeChecksum(header)) {
		dprintf(STDERR_FILENO, "Error: CheckSum does not match \n");
		return -1;
	}
	return bytesRead;
} 

static int skipContents(int archivefd, TarNode *tarNode)
{
	int contentSize = getContentsSize(tarNode);
	lseek(archivefd, contentSize, SEEK_CUR);
	return contentSize;
}

/* Function: Extract all files in tar archive
 * ------------------------------------------
 */
static int extractFiles(Params *params, TarNode *tarNode)
{
	int archivefd = open(params->archivePath, O_RDONLY);
	// int extractAll = params->filePaths == NULL;
	while (tarNode) {
		createFile(archivefd, tarNode);
		/*
		if (extractAll || searchFile(tarNode, params->PathNode)) 
		else 
			// offsetFildesPtr(archive, tarNode);
		*/
		tarNode = tarNode->next;
	}
	return 0;
}

static void createFile(int archivefd, TarNode *tarNode)
{
	char mode = tarNode->header->typeflag;
	switch (mode) {
	case REGTYPE:
	case AREGTYPE:
		printf("we reg file brahs\n");
		createREGTYPE(archivefd, tarNode);
		break;
	case LNKTYPE:
		printf("we link file brahs\n");
		createLNKTYPE(archivefd, tarNode);
		break;
	case SYMTYPE:
		printf("we symlink file brahs\n");
		createSYMTYPE(archivefd, tarNode);
		break;
	case DIRTYPE:
		printf("we dir file brahs\n");
		createDIRTYPE(archivefd, tarNode);
		break;
	}
}

static int countTrailingNulls(char *buffer, int contentsSize)
{
	int countNulls = 0;
	for (int i = contentsSize; buffer[i] == '\0' && i != 0; i--) {
		countNulls++;
	}
	return countNulls;
}

static int setFileInfo(const char *path, TarNode *tarNode)
{
	mode_t mode = _strtol(tarNode->header->mode, NULL, 8);
	chmod(path, mode);
	return (int) mode;
}

static int createREGTYPE(int archivefd, TarNode *tarNode)
{
	lseek(archivefd, BLOCKSIZE, SEEK_CUR);

	int contentsSize = getContentsSize(tarNode);
	char buffer[contentsSize];
	read(archivefd, buffer, contentsSize);
	int trailingNulls = countTrailingNulls(buffer, contentsSize);

	int filefd = open(tarNode->header->name, O_WRONLY | O_CREAT | O_TRUNC);
	setFileInfo(tarNode->header->name, tarNode);
	return write(filefd, buffer, contentsSize - trailingNulls);
}

static int createLNKTYPE(int archivefd, TarNode *tarNode)
{
	lseek(archivefd, BLOCKSIZE, SEEK_CUR);
	char *srcPath = tarNode->header->linkname;
	char *lnkPath = tarNode->header->name;
	if (link(srcPath, lnkPath) != 0) {
		return -1;
	}
	setFileInfo(tarNode->header->name, tarNode);
	return 0;
}

static int createSYMTYPE(int archivefd, TarNode *tarNode)
{
	lseek(archivefd, BLOCKSIZE, SEEK_CUR);
	char *srcPath = tarNode->header->linkname;
	char *lnkPath = tarNode->header->name;
	if (symlink(srcPath, lnkPath) != 0) {
		return -1;
	}
	setFileInfo(tarNode->header->name, tarNode);
	return 0;
}

static int createDIRTYPE(int archivefd, TarNode *tarNode)
{
	lseek(archivefd, BLOCKSIZE, SEEK_CUR);
	mkdir(tarNode->header->name, O_WRONLY | O_CREAT | O_TRUNC);
	setFileInfo(tarNode->header->name, tarNode);
	return 0;

}
