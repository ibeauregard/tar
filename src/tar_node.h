#ifndef TAR_NODE_H
#define TAR_NODE_H

typedef struct s_ParsedTar {
	PosixHeader *header;
	char *contents;
	struct s_ParsedTar *next;
} TarNode;

#endif
