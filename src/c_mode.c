#include "modes.h"
#include "tar_header.h"
#include "utils/_string.h"
#include "file/header_data.h"
#include "error/error.h"
#include "tar_node.h"
#include "tar_list.h"
#include <stdlib.h>
#include <dirent.h>

typedef struct dirent Dirent;

static int handlePath(char *path, TarList *list);
static int listEntry(HeaderData *headerData, TarList *list);
static void listHeader(HeaderData *headerData, TarList *list);
static int listDirEntries(const HeaderData *parsedDirHeader, TarList *list);
static char* buildPath(char* fullPath, const char* dirPath, const char* name);

int c_mode(Params *params)
{
	TarList list = getNewTarList();
	while (params->filePaths) {
		if (!_strcmp(params->archivePath, params->filePaths->path)) {
			error(FILE_IS_ARCHIVE_ERR, params->filePaths->path);
			return finalizeTarList(&list);
		}
		if (handlePath(params->filePaths->path, &list)) {
			return finalizeTarList(&list);
		}
		PathNode *current = params->filePaths;
		params->filePaths = params->filePaths->next;
		free(current);
	}
	return dumpToArchive(&list, params->archivePath);
}

int handlePath(char *path, TarList *list)
{
	HeaderData *headerData = malloc(sizeof (HeaderData));
	if (initHeaderData(headerData, path)) {
		return EXIT_FAILURE;
	}
	return listEntry(headerData, list);
}

int listEntry(HeaderData *headerData, TarList *list)
{
	listHeader(headerData, list);
	if (headerData->type == DIRTYPE) {
		return listDirEntries(headerData, list);
	}
	return EXIT_SUCCESS;
}

void listHeader(HeaderData *headerData, TarList *list)
{
	TarNode *node = getNewTarNode(headerData);
	if (!list->last) {
		list->node = list->last = node;
		return;
	}
	list->last = list->last->next = node;
}

int listDirEntries(const HeaderData *parsedDirHeader, TarList *list)
{
	DIR *folder = opendir(parsedDirHeader->path);
	Dirent *entry;
	while ((entry = readdir(folder))) {
		if (!_strcmp(entry->d_name, ".") || !_strcmp(entry->d_name, "..")) {
			continue;
		}
		char fullpath[_strlen(parsedDirHeader->path)
						+ _strlen(entry->d_name)
						+ 1];
		if (handlePath(buildPath(fullpath, parsedDirHeader->path, entry->d_name), list)) {
			return EXIT_FAILURE;
		}
	}
	closedir(folder);
	return EXIT_SUCCESS;
}

char* buildPath(char* fullPath, const char* dirPath, const char* name)
{
	return _strcat(_strcpy(fullPath, dirPath), name);
}
