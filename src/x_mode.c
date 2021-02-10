#include <stdio.h>            // For dprintf,
#include <stdlib.h>           // For malloc
#include <fcntl.h>            // For open
#include <unistd.h>           // For read, STDERR_FILENO

#include "tar_header.h"
#include "header_data.h"
#include "utils/_string.h"    // For _strlen
#include "utils/_stdlib.h"    // For _strtol
#include "modes.h"
#include "tar_node.h"
#include "tar_parsing.h"

#define MAXPATH 255

// Functions for parsing tar archive
static TarNode *newParsedTar();
static void freeParsedTar(TarNode *parsedTar);
static int checkEndOfArchive(int archivefd);
static int addNode(TarNode **headNode, TarNode **lastNode);
static int parseHeader(int archivefd, TarNode *lastNode);
static int skipContents(int archivefd, TarNode *lastNode);

// Functions for creating files
static int extractFiles(Params *params, TarNode *parsedTar);
static int shouldExtract(TarNode *tarNode, PathNode *PathNode);
static void createFile(int archivefd, TarNode *parsedTar);
static int createREGTYPE(int archivefd, TarNode *tarNode);
static int createLNKTYPE(int archivefd, TarNode *tarNode);
static int createSYMTYPE(int archivefd, TarNode *tarNode);
static int createDIRTYPE(int archivefd, TarNode *tarNode);

/* Function: Main function for x-mode
 * ----------------------------------
 * Parses tar achive and then extracts /creates all relevant files
 */ 
int x_mode(Params *params)
{
	TarNode *parsedTar = parseTar(params->archivePath);
	extractFiles(params, parsedTar);
	freeParsedTar(parsedTar);
	return EXIT_SUCCESS;
}

/* Function: Parses .tar archive into struct TarNode for individual files
 * ----------------------------------------------------------------------
 * parsedTar() parses all the headers of a tar archive and holds it in a  
 * TarNode structure. Each TarNode struct holds the header of a single file
 * along with a pointer to the next TarNode. Returns head of linked list.
 */ 
TarNode *parseTar(char *archivePath)
{
	int archivefd = open(archivePath, O_RDONLY);
	TarNode *headNode = NULL;
	TarNode *lastNode = headNode;
	do {
		if (checkEndOfArchive(archivefd))
			break;
		if (addNode(&headNode, &lastNode) == -1)
			return NULL;
		if (parseHeader(archivefd, lastNode) == -1)
			return NULL;
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
	newParsedTar->next = NULL;
	return newParsedTar;
}

static void freeParsedTar(TarNode *parsedTar) 
{
	TarNode *tmp;
	while (parsedTar) {
		free(parsedTar->header);
		tmp = parsedTar;
		parsedTar = parsedTar->next;
		free(tmp);
	}
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

/* Function: Takes a TarNode and returns number of bytes of contents
 * -----------------------------------------------------------------
 * The size of contents is rounded up to the nearest multiple of BLOCKSIZE
 */
static int getContentsSize(TarNode *tarNode)
{
	long size = _strtol(tarNode->header->size, NULL, 8);
	if (size == 0)
		return 0;
	int contentBlocks = (size - 1) / BLOCKSIZE + 1;
	int contentSize = contentBlocks * BLOCKSIZE;
	return contentSize;
}

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

/* Function: Moves pointer of fildes forward by BLOCKSIZE to skip header
 * ---------------------------------------------------------------------
 */
static int skipHeader(int archivefd)
{
	lseek(archivefd, BLOCKSIZE, SEEK_CUR);
	return BLOCKSIZE;
}

/* Function: Moves pointer of fildes forward to skip contents of file
 * ------------------------------------------------------------------
 */
static int skipContents(int archivefd, TarNode *tarNode)
{
	int contentSize = getContentsSize(tarNode);
	lseek(archivefd, contentSize, SEEK_CUR);
	return contentSize;
}

/* Function: Used to compare whether two pathnames are equivalent
 * --------------------------------------------------------------
 * Needs to account for pathNames that may or may not end in '/' 
 * but are otherwise still equivalent
 */
static int _pwdcmp(char *pathName1, char *pathName2)
{
	int len1 = _strlen(pathName1);
	int len2 = _strlen(pathName2);
	int result;
	if ((result = _strcmp(pathName1, pathName2)) == 0)
		return 0;
	if (*(pathName1 + len1-1) == '/' && *(pathName2 + len1-1) == '\0') {
		if (_strncmp(pathName1, pathName2, len1-1) == 0)
			return 0;
	}
	if (*(pathName1 + len2-1) == '\0' && *(pathName2 + len2-1) == '/') {
		if (_strncmp(pathName1, pathName2, len2-1) == 0)
			return 0;
	}
	return result;
}

/* Function: Returns 1 if file in tarNode ought to be extracted based on filePath
 * ------------------------------------------------------------------------------
 * NTD: This function is messy because it has to match a bunch of edge cases
 * and I'm not sure if there's a cleaner way of doing it.
 */
static int shouldExtract(TarNode *tarNode, PathNode *filePaths)
{
	char *argName = filePaths->path;
	char *tarName = tarNode->header->name;
	char buffer[MAXPATH] = { '\0' };
	int i;
	// We have this loop here so that parent dirs in pathname
	// of filePath will be found and created
	for (i = 0; i < (int) _strlen(argName) + 1; i++) {
		buffer[i] = *(argName + i);
		if (*(argName + i) == '/') {
			if (!_strncmp(tarName, buffer, _strlen(argName))) {
				return 1;
			}
		}
		// We have this here so that if dir in filePath arg 
		// does not contains '/', it will still match 
		if (*(tarName + i) == '/' && *(argName + i) == '\0') {
			if (!_strncmp(tarName, argName, _strlen(argName)-1)) 
				return 1;
		}
		// We have this block if filePath ends in '/' and
		// is referencing a file that doesn't end in '/'
		if (*(argName + i - 1) == '/' && *(tarName + i - 1) == '\0') {
			if (!_strncmp(tarName, argName, _strlen(argName)-1)) 
				return 1;
		}
	}
	// We have this block if filePath doesn't end in '/'
	// and is referencing a file that doesn't end in '/'
	if (!_strcmp(tarName, argName)) 
		return 1;
	return 0;
}

/* Function: Searches for argName in tar archive (via header->name's of TarNode)
 * -----------------------------------------------------------------------------
 */
static int findInTar(TarNode *tarNode, char *argName)
{
	while (tarNode) {
		if (_pwdcmp(tarNode->header->name, argName) == 0)
			return 1;
		tarNode = tarNode->next;
	}
	return 0;
}

/* Function: Extract files from tar archive
 * -----------------------------------------
 * This function controls the flow of extraction depending on whether all 
 * files ought to be extracted or whether only those files specified as 
 * arguments in the command line should be extracted. The latter requires
 * more plumbing.
 */
static int extractFiles(Params *params, TarNode *tarNode)
{
	int archivefd = open(params->archivePath, O_RDONLY);
	PathNode *argPaths = params->filePaths;
	int extractAll = (argPaths == NULL);
	if (extractAll) {
		while (tarNode) {
			createFile(archivefd, tarNode);
			tarNode = tarNode->next;
		}
		return 0;
	}
	while (argPaths) {
		if (!findInTar(tarNode, argPaths->path)) {
			dprintf(STDERR_FILENO, "%s: Not found in archive\n", 
			        argPaths->path);
		} else {
			TarNode *tarNodeLoop = tarNode;
			while(tarNodeLoop) {
				if (shouldExtract(tarNodeLoop, argPaths)) {
					createFile(archivefd, tarNodeLoop);
				} else {
					skipHeader(archivefd);
					skipContents(archivefd, tarNodeLoop);
				}
				tarNodeLoop = tarNodeLoop->next;
			}
		}
		argPaths = argPaths->next;
	}
	return 0;
}

/* Function: "Wrapper" function for managing which createX() function is used
 * --------------------------------------------------------------------------
 */
static void createFile(int archivefd, TarNode *tarNode)
{
	char mode = tarNode->header->typeflag;
	switch (mode) {
	case REGTYPE:
	case AREGTYPE:
		createREGTYPE(archivefd, tarNode);
		break;
	case LNKTYPE:
		createLNKTYPE(archivefd, tarNode);
		break;
	case SYMTYPE:
		createSYMTYPE(archivefd, tarNode);
		break;
	case DIRTYPE:
		createDIRTYPE(archivefd, tarNode);
		break;
	}
}

/* Function: Counts contiguous null chars starting at end of string and going back
 * -------------------------------------------------------------------------------
 * Because tar archive rounds up contents to nearests BLOCKSIZE, it fills
 * the remaining bytes with null chars. When we write to file, we don't 
 * want to include these chars. We count them here to remove them later.
 */
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
		dprintf(STDERR_FILENO, "%s: Cannot hard link to '%s'\n", 
		        lnkPath, srcPath);
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
		dprintf(STDERR_FILENO, "%s: Cannot soft link to '%s'\n", 
		        lnkPath, srcPath);
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
