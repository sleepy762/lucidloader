#include "shellerr.h"

void PrintCommandError(const char_t* cmd, const char_t* args, const uint8_t error)
{
    printf("%s: ", cmd);
    if (args != NULL)
    {
        printf("%s: ", args);
    }
    printf("%s\n", GetCommandErrorInfo(error));
}

const char_t* GetCommandErrorInfo(const uint8_t error)
{
    switch (error)
    {
        case CMD_SUCCESS:
        return "";

        case EPERM:
        return "operation not permitted.";

        case ENOENT:
        return "no such file or directory.";

        case ESRCH:
        return "no such process.";

        case EINTR:
        return "interrupted system call.";

        case EIO:
        return "i/o error.";

        case ENXIO:
        return "no such device or address.";

        case E2BIG:
        return "argument list too long.";

        case ENOEXEC:
        return "exec format error.";

        case EBADF:
        return "bad file number.";

        case ECHILD:
        return "no child processes.";

        case EAGAIN:
        return "try again.";

        case ENOMEM:
        return "out of memory.";

        case EACCES:
        return "permission denied.";

        case EFAULT:
        return "bad address.";

        case ENOTBLK:
        return "block device required.";

        case EBUSY:
        return "device or resource busy.";

        case EEXIST:
        return "file already exists.";

        case EXDEV:
        return "cross-device link.";

        case ENODEV:
        return "no such device.";

        case ENOTDIR:
        return "not a directory.";

        case EISDIR:
        return "is a directory.";

        case EINVAL:
        return "invalid argument.";

        case ENFILE:
        return "file table overflow.";

        case EMFILE:
        return "too many open files.";

        case ENOTTY:
        return "not a typewriter.";

        case ETXTBSY:
        return "text file busy.";

        case EFBIG:
        return "file too large.";

        case ENOSPC:
        return "no space left on device.";

        case ESPIPE:
        return "illegal seek.";

        case EROFS:
        return "read-only file system.";

        case EMLINK:
        return "too many links.";

        case EPIPE:
        return "broken pipe.";

        case EDOM:
        return "math argument out of domain of function.";

        case ERANGE:
        return "math result not representable.";

        case CMD_NO_FILE_SPECIFIED:
        return "no file name specified.";

        case CMD_NO_DIR_SPEFICIED:
        return "no directory specified.";

        case CMD_GENERAL_FILE_OPENING_ERROR:
        return "failed to open file.";

        case CMD_GENERAL_DIR_OPENING_ERROR:
        return "failed to open directory.";

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

        case CMD_QUOTATION_MARK_OPEN:
        return "no closing quotation mark found.";

        case CMD_REBOOT_FAIL:
        return "failed to reboot, check the log for more info.";

        case CMD_SHUTDOWN_FAIL:
        return "failed to shutdown, check the log for more info.";

        case CMD_REFUSE_REMOVE:
        return "refusing to remove '.' or '..' directory.";

        default:
        return "unknown error.";
    }
}
