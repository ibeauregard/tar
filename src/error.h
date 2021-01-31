#ifndef ERROR_H
#define ERROR_H

#define SYSCALL_ERR_CODE -1
#define STAT_ERR "my_tar: %s: Cannot stat: No such file or directory\n"
#define CANT_OPEN_FILE_ERR "my_tar: Cannot open %s\n"
#define CANT_READ_ERR "my_tar: Cannot read from %s\n"
#define CANT_WRITE_ERR "my_tar: Cannot write to %s\n"
#define FILE_IS_ARCHIVE_ERR "my_tar: %s: file is the archive; not dumped\n"

int error(const char *message, const char *messageArg);

#endif
