#include <stdlib.h>           // For EXIT_SUCCESS
#include <unistd.h>           // For STDERR_FILENO, STDIN_FILENO

#include "modes.h"
#include "tar_parsing.h"
#include "utils/_stdio.h"

// Functions for listing files
static void listContents(int archivefd, TarNode *parsedTar);

int t_mode(Params *params)
{
	if (params->archivePath == STDIN_FILENO) {
		_dprintf(STDERR_FILENO, "Refusing to read archive contents from terminal (missing -f option?)\n");
		return EXIT_FAILURE;
	}
	int status = 0;
	int applyParents = 0;
	TarNode *parsedTar = parseTar(params->archivePath, &status);
	if (!parsedTar) {
		_dprintf(STDERR_FILENO, 
		         "%s: Cannot open: No such file or directory\n",
		         params->archivePath);
		return EXIT_FAILURE;
	}
	applyTarNode(params, parsedTar, applyParents, listContents);
	freeParsedTar(parsedTar);
	return EXIT_SUCCESS;
}

static void listContents(int archivefd, TarNode *tarNode) {
	(void) archivefd;
	_printf("%s\n", tarNode->header->name);
}
