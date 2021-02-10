#ifndef TAR_PARSING_H
#define TAR_PARSING_H

#include "tar_node.h"

TarNode *parseTar(char *archivePath, int *status);

#endif
