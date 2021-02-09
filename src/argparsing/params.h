#ifndef PARAMS_H
#define PARAMS_H

#include "path_node.h"
#include "../mode.h"

typedef struct s_params
{
	Mode mode;
	char *archivePath;
	PathNode *filePaths;
} Params;

Params getNewParams();
int parseArguments(int n_arguments, char **arguments, Params *params);

#endif
