#ifndef PATH_NODE_H
#define PATH_NODE_H

typedef struct s_path_node
{
	char *path;
	struct s_path_node *next;
} PathNode;

#endif
