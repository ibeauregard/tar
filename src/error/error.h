#ifndef ERROR_H
#define ERROR_H

#include "../argparsing/params.h"

#define SYSCALL_ERR_CODE -1
#define STAT_ERR "my_tar: %s: Cannot stat: No such file or directory\n"
#define CANT_OPEN_FILE_ERR "my_tar: Cannot open %s\n"
#define CANT_READ_ERR "my_tar: Cannot read from %s\n"
#define CANT_WRITE_ERR "my_tar: Cannot write to %s\n"
#define FILE_IS_ARCHIVE_ERR "my_tar: %s: file is the archive; not dumped\n"
#define PARSE_ERROR_MESSAGE "my_tar: Internal error: Failed to parse mode in params.c\n"
#define PREVIOUS_ERROR_MESSAGE "my_tar: Exiting with failure status due to previous errors\n"

int error(const char *message, const char *messageArg);
int cleanupAfterFailure(Params *params);

#endif
