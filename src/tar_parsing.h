#ifndef TAR_PARSING_H
#define TAR_PARSING_H

#include "tar_node.h"

TarNode *parseTar(char *archivePath, int *status);
void freeParsedTar(TarNode *parsedTar);
int skipHeader(int archivefd);
int skipContents(int archivefd, TarNode *lastNode);
int getContentsSize(TarNode *tarNode);

#endif
