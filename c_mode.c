#include "modes.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define ARCHIVE_FLAGS O_CREAT|O_WRONLY|O_TRUNC
#define ARCHIVE_MODE S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH

int c_mode(Params *params)
{
	int archiveFD = open(params->archivePath, ARCHIVE_FLAGS, ARCHIVE_MODE);
	PathNode *pathNode = params->filePaths;
	while (pathNode) {
//		append(archiveFD, pathNode->path);
		PathNode *current = pathNode;
		pathNode = pathNode->next;
		free(current);
	}
	close(archiveFD);
	return 0;
}
