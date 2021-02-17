#include "modes.h"
#include "create.h"
#include "tar_parsing.h"
#include <stdlib.h>

static void destructExistingHeaders(TarNode *existingHeaders);

int u_mode(Params *params)
{
	int status;
	TarNode *existingHeaders = parseTar(params->archivePath, &status);
	if (!status) {
		status = create(params, true, existingHeaders);
	}
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
