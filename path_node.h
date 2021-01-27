#ifndef PATH_NODE_H
#define PATH_NODE_H

typedef struct s_path_node
{
	char *path;
	struct pathNode *next;
} PathNode;

#endif
