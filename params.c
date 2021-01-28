#include "params.h"
#include "_string.h"
#include "_stdio.h"
#include "path_node.h"
#include <stdlib.h>
#include <unistd.h>

#define STDOUT "-"
#define OPTION_PREFIX '-'
#define SEVERAL_OPTION_ERR_MESSAGE "my_tar: You may not specify more than one '-ctrux' option\n"
#define INVALID_OPTION_ERR_MESSAGE "my_tar: invalid option -- '%c'\n"
#define ARG_REQUIRED_ERR_MESSAGE "my_tar: option requires an argument -- '%c'\n"
#define MODE_UNDEFINED_ERR_MESSAGE "my_tar: You must specify one of the '-ctrux' options\n"
#define EMPTY_ARCHIVE_CREATION_ERR_MESSAGE "my_tar: Cowardly refusing to create an empty archive\n"
#define OPTION_INCOMPATIBILITY_ERR_MESSAGE "my_tar: Options '-ru' are incompatible with '-f -'\n"

typedef struct s_params_wrapper
{
	Params *params;
	PathNode *last;
	bool fArgExpected;
} ParamsWrapper;

static void initializeWrapper(ParamsWrapper *wrapper, Params *params);
static void initialize(Params *params);
static int handleArgument(char *argument, ParamsWrapper *wrapper);
static int handleOption(char* option, ParamsWrapper *wrapper);
static int handleOptionF(char nextOption, ParamsWrapper *wrapper);
static int setMode(Mode mode, Params *params);
static void updateLinks(ParamsWrapper *params, PathNode *node);
static int validate(const ParamsWrapper *wrapper);
static int argRequiredError(char option);

int parseArguments(int n_arguments, char **arguments, Params *params)
{
	ParamsWrapper wrapper;
	initializeWrapper(&wrapper, params);
	for (int i = 0; i < n_arguments; i++) {
		if (handleArgument(arguments[i], &wrapper)) {
			return EXIT_FAILURE;
		}
	}
	return validate(&wrapper);
}

void initializeWrapper(ParamsWrapper *wrapper, Params *params)
{
	initialize(params);
	wrapper->params = params;
	wrapper->last = NULL;
	wrapper->fArgExpected = false;
}

void initialize(Params *params)
{
	params->mode = UNDEFINED;
	params->archivePath = STDOUT;
	params->filePaths = NULL;
}

int handleArgument(char *argument, ParamsWrapper *wrapper)
{
	if (starts_with(argument, OPTION_PREFIX) && _strlen(argument) > 1) {
		return handleOption(argument + 1, wrapper);
	}
	if (wrapper->fArgExpected) {
		wrapper->params->archivePath = argument;
		wrapper->fArgExpected = false;
		return EXIT_SUCCESS;
	}
	updateLinks(wrapper, newPathNode(argument));
	return EXIT_SUCCESS;
}

int handleOption(char* option, ParamsWrapper *wrapper)
{
	switch (option[0]) {
		case 0:
			return EXIT_SUCCESS;
		case 'f':
			return handleOptionF(option[1], wrapper);
		case 'c':
			return setMode(C, wrapper->params) || handleOption(option + 1, wrapper);
		case 'r':
			return setMode(R, wrapper->params) || handleOption(option + 1, wrapper);
		case 't':
			return setMode(T, wrapper->params) || handleOption(option + 1, wrapper);
		case 'u':
			return setMode(U, wrapper->params) || handleOption(option + 1, wrapper);
		case 'x':
			return setMode(X, wrapper->params) || handleOption(option + 1, wrapper);
		default:
			_dprintf(STDERR_FILENO, INVALID_OPTION_ERR_MESSAGE, option[0]);
			return EXIT_FAILURE;
	}
}

int handleOptionF(char nextOption, ParamsWrapper *wrapper)
{
	wrapper->fArgExpected = true;
	if (nextOption) {
		return argRequiredError('f');
	}
	return EXIT_SUCCESS;
}

int setMode(Mode mode, Params *params)
{
	if (params->mode) {
		_dprintf(STDERR_FILENO, "%s", SEVERAL_OPTION_ERR_MESSAGE);
		return EXIT_FAILURE;
	}
	params->mode = mode;
	return EXIT_SUCCESS;
}

void updateLinks(ParamsWrapper *wrapper, PathNode *node)
{
	if (!wrapper->params->filePaths) {
		wrapper->params->filePaths = wrapper->last = node;
		return;
	}
	wrapper->last = wrapper->last->next = node;
}

int validate(const ParamsWrapper *wrapper)
{
	Params *params = wrapper->params;
	if (wrapper->fArgExpected) {
		return argRequiredError('f');
	}
	if (!params->mode) {
		_dprintf(STDERR_FILENO, "%s", MODE_UNDEFINED_ERR_MESSAGE);
		return EXIT_FAILURE;
	}
	if (params->mode == C && !params->filePaths) {
		_dprintf(STDERR_FILENO, "%s", EMPTY_ARCHIVE_CREATION_ERR_MESSAGE);
		return EXIT_FAILURE;
	}
	if ((params->mode == R || params->mode == U) && !_strcmp(params->archivePath, STDOUT)) {
		_dprintf(STDERR_FILENO, "%s", OPTION_INCOMPATIBILITY_ERR_MESSAGE);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int argRequiredError(char option)
{
	_dprintf(STDERR_FILENO, ARG_REQUIRED_ERR_MESSAGE, option);
	return EXIT_FAILURE;
}
