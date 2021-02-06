#include "modes.h"
#include "create.h"
#include "tar_parsing.h"
#include <stdlib.h>

static int sanityCheck(char *archivePath);

int r_mode(Params *params)
{
	return sanityCheck(params->archivePath) || create(params, true, NULL);
}

// TODO: should not always return EXIT_SUCCESS
int sanityCheck(char *archivePath)
{
	// TODO: Ask for interface change to TarNode *parseTar(char *archivePath, int *status);...
	// TODO: ... or int parseTar(char *archivePath, TarNode **headNode);
	TarNode *headNode = parseTar(archivePath);
	while (headNode) {
		TarNode *current = headNode;
		free(current->header);
		headNode = current->next;
		free(current);
	}
	return EXIT_SUCCESS;
}
