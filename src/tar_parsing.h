/* Helper Header File:
 * tar_parsing.c contains most of the logic for x-mode and t-mode. The main functions
 * are parsedTar(), which takes a tar archive and creates a linked list of tarNode
 * structures, and applyTarNode() which applies a fxn to each of the tarNodes in 
 * the linked list (i.e. createFile() for x-mode and printName() for t-mode).
 */

#ifndef TAR_PARSING_H
#define TAR_PARSING_H

#include "header_data.h"
#include "modes.h"            // For Params
#include "tar_node.h"         // For TarNode
#include "tar_parsing.h"      // For tarNode

TarNode *parseTar(char *archivePath, int *status);
int applyTarNode(Params *params, TarNode *tarNode, int applyParents,
                        void (*apply)(int archivefd, TarNode *tarNode));
int getContentsSize(TarNode *tarNode);
void freeParsedTar(TarNode *parsedTar);
#endif
