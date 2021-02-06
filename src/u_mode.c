#include "modes.h"
#include "create.h"
#include "tar_parsing.h"
#include <stdlib.h>

static void destructExistingHeaders(TarNode *existingHeaders);

int u_mode(Params *params)
{
	// TODO: Should not cause an error when archivePath points to an empty archive file
	TarNode *existingHeaders = parseTar(params->archivePath);
	int status = create(params, true, existingHeaders);
	destructExistingHeaders(existingHeaders);
	return status;
}

void destructExistingHeaders(TarNode *existingHeaders)
{
	while (existingHeaders) {
		TarNode *current = existingHeaders;
		free(current->header);
		existingHeaders = current->next;
		free(current);
	}
}
