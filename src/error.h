#ifndef ERROR_H
#define ERROR_H

#define SYSCALL_ERR_CODE -1
// Argument parsing errors
#define SEVERAL_OPTION_ERR "my_tar: You may not specify more than one '-ctrux' option\n"
#define INVALID_OPTION_ERR "my_tar: invalid option -- '%c'\n"
#define ARG_REQUIRED_ERR "my_tar: option requires an argument -- '%c'\n"
#define MODE_UNDEFINED_ERR "my_tar: You must specify one of the '-ctrux' options\n"
#define EMPTY_ARCHIVE_CREATION_ERR "my_tar: Cowardly refusing to create an empty archive\n"
#define OPTION_INCOMPATIBILITY_ERR "my_tar: Options '-ru' are incompatible with '-f -'\n"
// Tar errors
#define STAT_ERR "my_tar: %s: Cannot stat: No such file or directory\n"
#define CANT_OPEN_FILE_ERR "my_tar: Cannot open %s\n"
#define CANT_READ_ERR "my_tar: Cannot read from %s\n"
#define CANT_WRITE_ERR "my_tar: Cannot write to %s\n"
#define FILE_IS_ARCHIVE_ERR "my_tar: %s: file is the archive; not dumped\n"
// Main errors
#define PARSE_ERR "my_tar: Internal error: Failed to parse mode in params.c\n"
#define PREVIOUS_ERR "my_tar: Exiting with failure status due to previous errors\n"

int error(const char *message, const char *messageArg);
int errorCharArg(const char *message, char messageArg);

#endif
