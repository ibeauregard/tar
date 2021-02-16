#include <stdio.h>
#include <stdlib.h>           // For EXIT_SUCCESS
#include <unistd.h>           // For STDOUT_FILENO

#include "modes.h"
#include "tar_parsing.h"

// Functions for listing files
static void listContents(int archivefd, TarNode *parsedTar);

int t_mode(Params *params)
{
	int status = 0;
	int applyParents = 0;
	TarNode *parsedTar = parseTar(params->archivePath, &status);
	applyTarNode(params, parsedTar, applyParents, listContents);
	freeParsedTar(parsedTar);
	return EXIT_SUCCESS;
}

static void listContents(int archivefd, TarNode *tarNode) {
	(void) archivefd;
	printf("%s\n", tarNode->header->name);
}
