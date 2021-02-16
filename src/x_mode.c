#include <stdio.h>            // For dprintf
#include <stdlib.h>           // For malloc, lseek
#include <fcntl.h>            // For open
#include <unistd.h>           // For read, STDERR_FILENO

#include "utils/_stdlib.h"    // For _strtol
#include "tar_parsing.h"

// Functions for creating files
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
	int status = 0;
	int applyParents = 1;
	TarNode *parsedTar = parseTar(params->archivePath, &status);
	applyTarNode(params, parsedTar, applyParents, createFile);
	freeParsedTar(parsedTar);
	return EXIT_SUCCESS;
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
	for (int i = contentsSize-1; buffer[i] == '\0' && i != 0; i--) {
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
