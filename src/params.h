#ifndef PARAMS_H
#define PARAMS_H

#include "argparsing/path_node.h"
#include "mode.h"

typedef struct s_params
{
	Mode mode;
	char *archivePath;
	PathNode *filePaths;
} Params;

Params getNewParams(void);
int parseArguments(int n_arguments, char **arguments, Params *params);
int cleanupAfterFailure(Params *params);

#endif
