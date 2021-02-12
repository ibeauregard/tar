#include <stdio.h>            // For drpintf
#include <stdlib.h>           // For malloc
#include <fcntl.h>            // For open
#include <unistd.h>           // For lseek

#include "utils/_stdlib.h"
#include "tar_parsing.h"

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
		dprintf(STDERR_FILENO, "Error: Cannot check end of archive\n");
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
		dprintf(STDERR_FILENO, "Error: Cannot read BLOCKSIZE bytes\n");
		lseek(archivefd, -bytesRead, SEEK_CUR);
		return -1;
	}
	if (_strtol(header->chksum, NULL, 8) != computeChecksum(header)) {
		dprintf(STDERR_FILENO, 
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

/* Function: Moves pointer of fildes forward by BLOCKSIZE to skip header
 * ---------------------------------------------------------------------
 */
int skipHeader(int archivefd)
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
int skipContents(int archivefd, TarNode *tarNode)
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
