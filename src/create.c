#include "create.h"
#include "tar_header.h"
#include "utils/_string.h"
#include "header_data.h"
#include "error.h"
#include "tar_list.h"
#include <stdlib.h>
#include <dirent.h>

typedef struct dirent Dirent;

static int handlePath(char *path, TarList *list, TarNode *existingHeaders);
static bool needsUpdate(const char *path, time_t mtime, TarNode *existingHeaders);
static int listEntry(HeaderData *headerData, TarList *list, TarNode *existingHeaders);
static HeaderData *previousLink(Stat *fileStat, TarList *list);
static void listHeader(HeaderData *headerData, TarList *list);
static int listDirEntries(const HeaderData *dirHeaderData, TarList *list, TarNode *existingHeaders);
static char* buildPath(char* fullPath, const char* dirPath, const char* name);

int create(Params *params, bool append, TarNode *existingHeaders)
{
	TarList list = getNewTarList();
	while (params->filePaths) {
		PathNode *current = params->filePaths;
		if (!_strcmp(params->archivePath, current->path)) {
			error(FILE_IS_ARCHIVE_ERR, current->path);
			return finalizeTarList(&list);
		}
		if (handlePath(current->path, &list, existingHeaders)) {
			return finalizeTarList(&list);
		}
		params->filePaths = current->next;
		free(current);
	}
	return dumpToArchive(&list, params, append);
}

int handlePath(char *path, TarList *list, TarNode *existingHeaders)
{
	Stat fileStat;
	if (lstat(path, &fileStat) == SYSCALL_ERR_CODE) {
		return error(STAT_ERR, path);
	}
	if (!needsUpdate(path, fileStat.st_mtime, existingHeaders)) {
		return EXIT_SUCCESS;
	}
	HeaderData *headerData = getHeaderData(&fileStat, path, previousLink(&fileStat, list));
	return listEntry(headerData, list, existingHeaders);
}

bool needsUpdate(const char *path, time_t mtime, TarNode *existingHeaders)
{
	while (existingHeaders) {
		PosixHeader *header = existingHeaders->header;
		char headerName[255];
		getNameFromHeader(header, headerName);
		if (_strcmp(path, headerName)) {
			continue;
		}
		if (mtime <= getMtimeFromHeader(header)) {
			return false;
		}
		existingHeaders = existingHeaders->next;
	}
	return true;
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

int listEntry(HeaderData *headerData, TarList *list, TarNode *existingHeaders)
{
	listHeader(headerData, list);
	if (headerData->type == DIRTYPE) {
		return listDirEntries(headerData, list, existingHeaders);
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

int listDirEntries(const HeaderData *dirHeaderData, TarList *list, TarNode *existingHeaders)
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
		if (handlePath(buildPath(fullpath, dirHeaderData->name, entry->d_name), list, existingHeaders)) {
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
