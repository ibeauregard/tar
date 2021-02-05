#include "modes.h"
#include "tar_header.h"
#include "utils/_string.h"
#include "header_data.h"
#include "error/error.h"
#include "tar_node.h"
#include "tar_list.h"
#include <stdlib.h>
#include <dirent.h>

typedef struct dirent Dirent;

static int handlePath(char *path, TarList *list);
static int listEntry(HeaderData *headerData, TarList *list);
static HeaderData *previousLink(Stat *fileStat, TarList *list);
static void listHeader(HeaderData *headerData, TarList *list);
static int listDirEntries(const HeaderData *dirHeaderData, TarList *list);
static char* buildPath(char* fullPath, const char* dirPath, const char* name);

int c_mode(Params *params)
{
	TarList list = getNewTarList();
	while (params->filePaths) {
		PathNode *current = params->filePaths;
		if (!_strcmp(params->archivePath, current->path)) {
			error(FILE_IS_ARCHIVE_ERR, current->path);
			return finalizeTarList(&list);
		}
		if (handlePath(current->path, &list)) {
			return finalizeTarList(&list);
		}
		params->filePaths = current->next;
		free(current);
	}
	return dumpToArchive(&list, params->archivePath);
}

int handlePath(char *path, TarList *list)
{
	Stat fileStat;
	if (lstat(path, &fileStat) == SYSCALL_ERR_CODE) {
		return error(STAT_ERR, path);
	}
	HeaderData *headerData = getHeaderData(&fileStat, path, previousLink(&fileStat, list));
	return listEntry(headerData, list);
}

HeaderData *previousLink(Stat *fileStat, TarList *list)
{
	TarNode *node = list->first;
	while (node) {
		if (fileStat->st_dev == node->headerData->deviceNumber
			&& fileStat->st_ino == node->headerData->inodeNumber) {
			return node->headerData;
		}
		node = node->next;
	}
	return NULL;
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
		*(TarNode **)&list->first = list->last = node;
		return;
	}
	list->last = list->last->next = node;
}

int listDirEntries(const HeaderData *dirHeaderData, TarList *list)
{
	DIR *folder = opendir(dirHeaderData->name);
	Dirent *entry;
	while ((entry = readdir(folder))) {
		if (!_strcmp(entry->d_name, ".") || !_strcmp(entry->d_name, "..")) {
			continue;
		}
		char fullpath[_strlen(dirHeaderData->name)
						+ _strlen(entry->d_name)
						+ 1];
		if (handlePath(buildPath(fullpath, dirHeaderData->name, entry->d_name), list)) {
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
