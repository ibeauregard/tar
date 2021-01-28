#include "params.h"
#include "_string.h"
#include "path_node.h"
#include <stdlib.h>

#define STDOUT "-"
#define OPTION_PREFIX '-'

typedef struct s_params_wrapper
{
	Params *params;
	PathNode *last;
	bool fArgExpected;
} ParamsWrapper;

static void initialize_wrapper(ParamsWrapper *wrapper, Params *params);
static void initialize(Params *params);
static int handle_argument(char *argument, ParamsWrapper *wrapper);
static int handle_option(char* option, ParamsWrapper *wrapper);
//static int set_mode(Mode mode, Params *params);
static void update_links(ParamsWrapper *params, PathNode *node);

int parse_arguments(int n_arguments, char **arguments, Params *params)
{
	ParamsWrapper wrapper;
	initialize_wrapper(&wrapper, params);
	for (int i = 0; i < n_arguments; i++) {
		if (handle_argument(arguments[i], &wrapper)) {
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

void initialize_wrapper(ParamsWrapper *wrapper, Params *params)
{
	initialize(params);
	wrapper->params = params;
	wrapper->last = NULL;
	wrapper->fArgExpected = false;
}

void initialize(Params *params)
{
	params->mode = 0;
	params->archivePath = STDOUT;
	params->filePaths = NULL;
}

int handle_argument(char *argument, ParamsWrapper *wrapper)
{
	if (starts_with(argument, OPTION_PREFIX) && _strlen(argument) > 1) {
		return handle_option(argument + 1, wrapper);
	}
	update_links(wrapper, newPathNode(argument));
	return EXIT_SUCCESS;
}

int handle_option(char* option, ParamsWrapper *wrapper)
{
	switch (option[0]) {
		case 0:
			return EXIT_SUCCESS;
		case 'c':
			wrapper->params->mode = C;
			return handle_option(option + 1, wrapper);
		case 'r':
			wrapper->params->mode = R;
			return handle_option(option + 1, wrapper);
		case 't':
			wrapper->params->mode = T;
			return handle_option(option + 1, wrapper);
		case 'u':
			wrapper->params->mode = U;
			return handle_option(option + 1, wrapper);
		case 'x':
			wrapper->params->mode = X;
			return handle_option(option + 1, wrapper);
		default:
			return 1;
	}
}

//int set_mode(Mode mode, Params *params)
//{
//
//}

void update_links(ParamsWrapper *wrapper, PathNode *node)
{
	if (!wrapper->params->filePaths) {
		wrapper->params->filePaths = wrapper->last = node;
		return;
	}
	wrapper->last = wrapper->last->next = node;
}
