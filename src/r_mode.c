#include "modes.h"
#include "create.h"
#include "tar_parsing.h"
#include <stdlib.h>

static int sanityCheck(char *archivePath);

int r_mode(Params *params)
{
	return sanityCheck(params->archivePath) || create(params, true, NULL);
}

int sanityCheck(char *archivePath)
{
	int status;
	TarNode *headNode = parseTar(archivePath, &status);
	while (headNode) {
		TarNode *current = headNode;
		free(current->header);
		headNode = current->next;
		free(current);
	}
	return status;
}
