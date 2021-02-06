#include "modes.h"
#include "create.h"
#include "tar_parsing.h"
#include <stdlib.h>

static void destructExistingHeader(TarNode *existingHeaders);

int u_mode(Params *params)
{
	// TODO: Should not cause an error when archivePath points to an empty archive file
	TarNode *existingHeaders = parseTar(params->archivePath);
	int status = create(params, true, existingHeaders);
	destructExistingHeader(existingHeaders);
	return status;
}

void destructExistingHeader(TarNode *existingHeaders)
{
	while (existingHeaders) {
		TarNode *current = existingHeaders;
		free(current->header);
		existingHeaders = current->next;
		free(current);
	}
}
