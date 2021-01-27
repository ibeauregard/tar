#ifndef PARAMS_H
#define PARAMS_H

#include "path_node.h"

typedef struct s_params
{
	Mode mode;
	char *archivePath;
	PathNode *filePaths;
} Params;

#endif
