#include <stdlib.h>           // For malloc
#include <fcntl.h>            // For open
#include <unistd.h>           // For lseek, STDERR_FILENO

#include "utils/_stdlib.h"
#include "utils/_stdio.h"
#include "utils/_string.h"    // For _strlen
#include "tar_parsing.h"

#define MAXPATH 255

static TarNode *newParsedTar()
{
	TarNode *newParsedTar = malloc(sizeof(TarNode));
	if (!newParsedTar)
		return NULL;
	newParsedTar->header = NULL;
	newParsedTar->next = NULL;
	return newParsedTar;
}

void freeParsedTar(TarNode *parsedTar) 
{
	TarNode *tmp;
	while (parsedTar) {
		free(parsedTar->header);
		tmp = parsedTar;
		parsedTar = parsedTar->next;
		free(tmp);
	}
}

/* Function: Checks if file pointed to by archivefd is empty
 * ---------------------------------------------------------
 */
static int archiveIsEmpty(int archivefd)
{
	int returnValue = 0;
	if (lseek(archivefd, 0, SEEK_END) == 0)
		returnValue = 1;
	lseek(archivefd, 0, SEEK_SET);
	return returnValue;
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
		_dprintf(STDERR_FILENO, "Error: Cannot check end of archive\n");
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

/* Function: Parses header in .tar file and puts it in PosixHeader struct
 * ----------------------------------------------------------------------
 */
static int parseHeader(int archivefd, TarNode *lastNode)
{
	PosixHeader *header = malloc(sizeof(PosixHeader));
	lastNode->header = header;
	int bytesRead = read(archivefd, header, BLOCKSIZE);
	if (bytesRead < BLOCKSIZE) {
		_dprintf(STDERR_FILENO, "Error: Cannot read BLOCKSIZE bytes\n");
		lseek(archivefd, -bytesRead, SEEK_CUR);
		return -1;
	}
	if (_strtol(header->chksum, NULL, 8) != computeChecksum(header)) {
		_dprintf(STDERR_FILENO, 
		        "Error: chksum does not match, may be invalid archive.\n");
		return -1;
	}
	return bytesRead;
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
		_dprintf(STDERR_FILENO, "Error: Could not allocate memory\n");
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

/* Function: Moves pointer of fildes forward by BLOCKSIZE to skip header
 * ---------------------------------------------------------------------
 */
static int skipHeader(int archivefd)
{
	lseek(archivefd, BLOCKSIZE, SEEK_CUR);
	return BLOCKSIZE;
}

/* Function: Takes a TarNode and returns number of bytes of contents
 * -----------------------------------------------------------------
 * The size of contents is rounded up to the nearest multiple of BLOCKSIZE
 */
int getContentsSize(TarNode *tarNode)
{
	long size = _strtol(tarNode->header->size, NULL, 8);
	if (size == 0)
		return 0;
	int contentBlocks = (size - 1) / BLOCKSIZE + 1;
	int contentSize = contentBlocks * BLOCKSIZE;
	return contentSize;
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

/* Function: Parses .tar archive into struct TarNode for individual files
 * ----------------------------------------------------------------------
 * parsedTar() parses all the headers of a tar archive and holds it in a  
 * TarNode structure. Each TarNode struct holds the header of a single file
 * along with a pointer to the next TarNode. Returns head of linked list.
 */ 
TarNode *parseTar(char *archivePath, int *status)
{
	int archivefd = open(archivePath, O_RDONLY);
	TarNode *headNode = NULL;
	TarNode *lastNode = headNode;
	*status = 0;
	if (archivefd == -1 || archiveIsEmpty(archivefd)) 
		return NULL;
	do {
		int res = checkEndOfArchive(archivefd);
		if (res == -1) {
			*status = 1;
			return NULL;
		} 
		if (res == 1)
			break;
		if (addNode(&headNode, &lastNode) == -1) {
			*status = 1;
			return NULL;
		}
		if (parseHeader(archivefd, lastNode) == -1) {
			*status = 1;
			return NULL;
		}
		skipContents(archivefd, lastNode);
	} while (true);
	return headNode;
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

/* Function: Returns copy of str but with all trailing dchar removed
 * -----------------------------------------------------------------
 */
static char *removeTrailingChar(char *str, char dchar) 
{
	int strLength = _strlen(str);
	char *strClean = malloc(strLength + 1);
	_strcpy(strClean, str);
	*(strClean + strLength--) = '\0';
	while (strLength >= 0 && strClean[strLength] == dchar) {
		strClean[strLength] = '\0';
	}
	return strClean;
}

/* Function: Returns 1 if file in tarNode ought to be extracted based on filePath
 * ------------------------------------------------------------------------------
 * NTD: This function is messy because it has to match a bunch of edge cases
 * and I'm not sure if there's a cleaner way of doing it.
 */
static int shouldApply(TarNode *tarNode, PathNode *filePaths, int applyParents)
{
	char *argName = removeTrailingChar(filePaths->path, '/');
	char *tarName = tarNode->header->name;

	char buffer[MAXPATH] = { '\0' };
	int i;
	// We have this loop here so that parent dirs in pathname
	// of filePath will be found and created
	for (i = 0; i < (int) _strlen(argName) + 1; i++) {
		buffer[i] = *(argName + i);
		// We have this block for when 1) we want parent dir and
		// 2) if we want all nested / sub-files or directories
		if ((applyParents && *(tarName + i) != '\0')
		                  && *(argName + i) == '/') {
			if (!_strncmp(tarName, buffer, _strlen(argName))) {
				free(argName);
				return 1;
			}
		}
		// We have this here so that if dir in filePath arg 
		// does not contains '/', it will still match 
		if (*(tarName + i) == '/' && *(argName + i) == '\0') { 
			if (!_strncmp(tarName, argName, _strlen(argName)-1)) {
				free(argName);
				return 1;
			}
		}
		// We have this block if filePath ends in '/' and
		// is referencing a file that doesn't end in '/'
		if (*(argName + i-1) == '/' && *(tarName + i-1) == '\0') {
			if (!_strncmp(tarName, argName, _strlen(argName)-1)) {
				free(argName);
				return 1;
			}
		}
	}
	// We have this block if filePath doesn't end in '/'
	// and is referencing a file that doesn't end in '/'
	if (!_strcmp(tarName, argName)) {
		free(argName);
		return 1;
	}
	free(argName);
	return 0;
}

/* Function: Extract files from tar archive
 * -----------------------------------------
 * Originally this function was used for extraction (looping through TarNode
 * and applying createFile functon). But because logic needs to be reused for
 * t-mode, decided to generalize the function and allow one to pass in a pointer
 * to a function. Thus, for x-mode the createFile function would be used and
 * for t-mode another function will be used. 
 *
 * This function selectively applies the (*apply) function depending on whether 
 * all tarNodes ought to be applied to fxn or whether only those files specified 
 * as arguments in the command line (i.e. Params *params) should be extracted.
 * @applyPar: Indicates whether (*apply) function should also apply to parent
 *            directories of the pathname
 */
int applyTarNode(Params *params, TarNode *tarNode, int applyPar,
                        void (*apply)(int archivefd, TarNode *tarNode))
{
	int archivefd = open(params->archivePath, O_RDONLY);
	PathNode *argPaths = params->filePaths;
	int extractAll = (argPaths == NULL);
	if (extractAll) {
		while (tarNode) {
			(*apply)(archivefd, tarNode);
			tarNode = tarNode->next;
		}
		return 0;
	}
	while (argPaths) {
		if (!findInTar(tarNode, argPaths->path)) {
			_dprintf(STDERR_FILENO, "%s: Not found in archive\n", 
			        argPaths->path);
		} else {
			TarNode *tarNodeLoop = tarNode;
			while(tarNodeLoop) {
				if ((shouldApply(tarNodeLoop, argPaths, applyPar))) {
					(*apply)(archivefd, tarNodeLoop);
				} else {
					skipHeader(archivefd);
					skipContents(archivefd, tarNodeLoop);
				}
				tarNodeLoop = tarNodeLoop->next;
			}
		}
		PathNode *tmp = argPaths;
		argPaths = argPaths->next;
		free(tmp);
	}
	return 0;
}
