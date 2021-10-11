#include "shellerr.h"

void PrintCommandError(const char* cmd, int error)
{
    printf("\n%s: %s", cmd, GetCommandErrorInfo(error));
}

const char* GetCommandErrorInfo(int error)
{
    switch (error)
    {
        case CMD_SUCCESS:
        return "";

        case CMD_NO_FILE_SPECIFIED:
        return "no file name specified.";

        case CMD_NO_DIR_SPEFICIED:
        return "no directory specified.";

        case CMD_GENERAL_FILE_OPENING_ERROR:
        return "failed to open file.";

        case CMD_GENERAL_DIR_OPENING_ERROR:
        return "failed to open directory.";

        case CMD_READ_ONLY_FILESYSTEM:
        return "permission denied - readonly filesystem.";

        case CMD_DIR_ALREADY_EXISTS:
        return "directory already exists.";

        case CMD_DIR_NOT_FOUND:
        return "directory not found.";

        case CMD_CANT_READ_DIR:
        return "unable to read directory.";

        case CMD_OUT_OF_MEMORY:
        return "failed to allocate memory.";

        case CMD_NOT_FOUND:
        return "command not found.";

        case CMD_BRIEF_HELP_NOT_AVAILABLE:
        return "no brief help available.";

        case CMD_LONG_HELP_NOT_AVAILABLE:
        return "no long help available.";

        default:
        return "unknown error.";
    }
}
