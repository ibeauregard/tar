#ifndef ARCHIVE_FILE_H
#define ARCHIVE_FILE_H

typedef struct s_archive {
	const char *path;
	int fd;
} Archive;

int initArchive(Archive *archive, const char *archivePath);
int appendEnd(Archive *archive);
int finalizeArchive(Archive *archive);

#endif