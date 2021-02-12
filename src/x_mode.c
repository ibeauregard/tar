#include <stdio.h>            // For dprintf
#include <stdlib.h>           // For malloc, lseek
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

// Functions for creating files
static int applyTarNode(Params *params, TarNode *parsedTar,
                        void (*apply)(int archivefd, TarNode *tarNode));
static int shouldApply(TarNode *tarNode, PathNode *PathNode);
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
	TarNode *parsedTar = parseTar(params->archivePath, &status);
	applyTarNode(params, parsedTar, createFile);
	freeParsedTar(parsedTar);
	return EXIT_SUCCESS;
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
static int shouldApply(TarNode *tarNode, PathNode *filePaths)
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
 * Originally this function was used for extraction (looping through TarNode
 * and applying createFile functon). But because logic needs to be reused for
 * t-mode, decided to generalize the function and allow one to pass in a pointer
 * to a function. Thus, for x-mode the createFile function would be used and
 * for t-mode another function will be used. 
 *
 * This function selectively applies the (*apply) function depending on whether 
 * all tarNodes ought to be applied to fxn or whether only those files specified 
 * as arguments in the command line (i.e. Params *params) should be extracted.
 */
static int applyTarNode(Params *params, TarNode *tarNode, 
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
			dprintf(STDERR_FILENO, "%s: Not found in archive\n", 
			        argPaths->path);
		} else {
			TarNode *tarNodeLoop = tarNode;
			while(tarNodeLoop) {
				if (shouldApply(tarNodeLoop, argPaths)) {
					(*apply)(archivefd, tarNode);
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
