#ifndef ARCHIVE_FILE_H
#define ARCHIVE_FILE_H

typedef struct s_archive_file {
	char *path;
	int fd;
} ArchiveFile;

#endif
