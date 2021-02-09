#include "params.h"
#include "../utils/_string.h"
#include "../utils/_stdio.h"
#include "path_node.h"
#include "../constants.h"
#include <stdlib.h>
#include <unistd.h>

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

static ParamsWrapper getNewParamsWrapper(Params *params);
static int handleArgument(char *argument, ParamsWrapper *wrapper);
static int handleOptions(char* options, ParamsWrapper *wrapper);
static int handleOptionF(char nextOption, ParamsWrapper *wrapper);
static int setMode(Mode mode, ParamsWrapper *wrapper, char *options);
static void updateLinks(ParamsWrapper *wrapper, PathNode *node);
static int validate(const ParamsWrapper *wrapper);
static int argRequiredError(char option);

Params getNewParams()
{
	Params params = {
		.archivePath = STDOUT_PATH,
		.mode = UNDEFINED,
		.filePaths = NULL
	};
	return params;
}

int parseArguments(int n_arguments, char **arguments, Params *params)
{
	ParamsWrapper wrapper = getNewParamsWrapper(params);
	for (int i = 0; i < n_arguments; i++) {
		if (handleArgument(arguments[i], &wrapper)) {
			return EXIT_FAILURE;
		}
	}
	return validate(&wrapper);
}

ParamsWrapper getNewParamsWrapper(Params *params)
{
	ParamsWrapper wrapper = {
		.params = params,
		.last = NULL,
		.fArgExpected = false
	};
	return wrapper;
}

int handleArgument(char *argument, ParamsWrapper *wrapper)
{
	if (starts_with(argument, OPTION_PREFIX) && _strlen(argument) > 1) {
		return handleOptions(argument + 1, wrapper);
	}
	if (wrapper->fArgExpected) {
		wrapper->params->archivePath = argument;
		wrapper->fArgExpected = false;
		return EXIT_SUCCESS;
	}
	updateLinks(wrapper, newPathNode(argument));
	return EXIT_SUCCESS;
}

int handleOptions(char* options, ParamsWrapper *wrapper)
{
	switch (options[0]) {
		case 0:
			return EXIT_SUCCESS;
		case 'f':
			return handleOptionF(options[1], wrapper);
		case 'c':
			return setMode(C, wrapper, options);
		case 'r':
			return setMode(R, wrapper, options);
		case 't':
			return setMode(T, wrapper, options);
		case 'u':
			return setMode(U, wrapper, options);
		case 'x':
			return setMode(X, wrapper, options);
		default:
			_dprintf(STDERR_FILENO, INVALID_OPTION_ERR_MESSAGE, options[0]);
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

int setMode(Mode mode, ParamsWrapper *wrapper, char *options)
{
	if (wrapper->params->mode && wrapper->params->mode != mode) {
		_dprintf(STDERR_FILENO, "%s", SEVERAL_OPTION_ERR_MESSAGE);
		return EXIT_FAILURE;
	}
	wrapper->params->mode = mode;
	return handleOptions(options + 1, wrapper);
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
	if (wrapper->fArgExpected) {
		return argRequiredError('f');
	}
	Params *params = wrapper->params;
	if (!params->mode) {
		_dprintf(STDERR_FILENO, "%s", MODE_UNDEFINED_ERR_MESSAGE);
		return EXIT_FAILURE;
	}
	if (params->mode == C && !params->filePaths) {
		_dprintf(STDERR_FILENO, "%s", EMPTY_ARCHIVE_CREATION_ERR_MESSAGE);
		return EXIT_FAILURE;
	}
	if ((params->mode == R || params->mode == U) && !_strcmp(params->archivePath, STDOUT_PATH)) {
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
