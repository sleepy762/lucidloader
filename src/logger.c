#include "logger.h"
#include "shellutils.h"
#include "version.h"

#define LOG_PATH        ("\\EFI\\ezboot\\ezboot-log.txt")
#define OLD_LOG_PATH    ("\\EFI\\ezboot\\ezboot-log.txt.old")

#define SECONDS_IN_DAY (86400)
#define SECONDS_IN_HOUR (3600)
#define SECONDS_IN_MINUTE (60)

static efi_time_t timeSinceInit = {0};


// Creates an empty log file and initializes the timeSinceInit variable
// Returns 1 on success and 0 on failure
int8_t InitLogger(void)
{
    // If a log file already exists, keep it as an old log
    FILE* fp = fopen(LOG_PATH, "r");
    if (fp != NULL)
    {
        fclose(fp);
        CopyFile(LOG_PATH, OLD_LOG_PATH);
    }

    fp = fopen(LOG_PATH, "w");
    if (fp != NULL)
    {
        fclose(fp);
        RT->GetTime(&timeSinceInit, NULL);

        // Print the date of the log and the version of the bootloader
        Log(LL_INFO, 0, "Starting %s v%s", EZBOOT_NAME_STR, EZBOOT_VERSION);
        Log(LL_INFO, 0, "Log date: %02d/%02d/%04d %02d:%02d:%02d.", 
            timeSinceInit.Day, timeSinceInit.Month, timeSinceInit.Year,
            timeSinceInit.Hour, timeSinceInit.Minute, timeSinceInit.Second);

        return 1;
    }
    return 0;
}

// The status parameter is optional and can be set to 0 if unneeded
// fmtMessage should be a string literal (with optional formatting like printf)
// the last ... are for formatting fmtMessage
void Log(log_level_t loglevel, efi_status_t status, const char_t* fmtMessage, ...)
{
    FILE* log = fopen(LOG_PATH, "a");
    // Don't log if the logger hasn't been initialized (or file is not writable)
    if (log == NULL || timeSinceInit.Day == 0)
    {
        return;
    }
    
    // Print the seconds since launch and log level
    fprintf(log, "[%04ds] [%s] ", GetSecondsSinceInit(), LogLevelString(loglevel));

    // Print the string and add formatting (if there is any)
    va_list args;
    va_start(args, fmtMessage);
    vfprintf(log, fmtMessage, args);
    va_end(args);

    // Append a UEFI error message if the status argument is an error status
    if (EFI_ERROR(status))
    {
        fprintf(log, " (EFI Error: %s)", EfiErrorString(status));
    }

    fprintf(log, "\n");
    fclose(log);
}

time_t GetSecondsSinceInit(void)
{
    efi_time_t currTime = {0};
    efi_status_t status = RT->GetTime(&currTime, NULL);
    if (EFI_ERROR(status))
    {
        return 0;
    }

    time_t seconds = 0;
    seconds += (currTime.Day - timeSinceInit.Day) * SECONDS_IN_DAY;
    seconds += (currTime.Hour - timeSinceInit.Hour) * SECONDS_IN_HOUR;
    seconds += (currTime.Minute - timeSinceInit.Minute) * SECONDS_IN_MINUTE;
    seconds += currTime.Second - timeSinceInit.Second;
    
    return seconds;
}

void PrintLogFile(void)
{
    uint8_t res = PrintFileContent(LOG_PATH);
    if (res != 0)
    {
        printf("Failed to open log file!\n");
    }
}

const char_t* LogLevelString(log_level_t loglevel)
{
    switch (loglevel)
    {
        case LL_INFO:
        return "INFO";

        case LL_WARNING:
        return "WARNING";

        case LL_ERROR:
        return "ERROR";

        default:
        return "UNKNOWN";
    }
}

// More informative error messages for each UEFI error status
const char_t* EfiErrorString(efi_status_t status)
{
    switch(status)
    {
        case EFI_SUCCESS:
        return "";
        
        case EFI_LOAD_ERROR:
        return "The image failed to load.";

        case EFI_INVALID_PARAMETER:
        return "A parameter was incorrect.";

        case EFI_UNSUPPORTED:
        return "The operation is not supported.";

        case EFI_BAD_BUFFER_SIZE:
        return "The buffer was not the proper size for the request.";

        case EFI_BUFFER_TOO_SMALL:
        return "The buffer is not large enough to hold the requested data.";

        case EFI_NOT_READY:
        return "There is no data pending upon return.";

        case EFI_DEVICE_ERROR:
        return "The physical device reported an error while attempting the operation.";

        case EFI_WRITE_PROTECTED:
        return "The device cannot be written to.";

        case EFI_OUT_OF_RESOURCES:
        return "A resource has run out.";

        case EFI_VOLUME_CORRUPTED:
        return "An inconstancy was detected on the file system causing the operating to fail.";

        case EFI_VOLUME_FULL:
        return "There is no more space on the file system.";

        case EFI_NO_MEDIA:
        return "The device does not contain any medium to perform the operation.";

        case EFI_MEDIA_CHANGED:
        return "The medium in the device has changed since the last access.";

        case EFI_NOT_FOUND:
        return "The item was not found.";

        case EFI_ACCESS_DENIED:
        return "Access was denied.";

        case EFI_NO_RESPONSE:
        return "The server was not found or did not respond to the request";

        case EFI_NO_MAPPING:
        return "A mapping to a device does not exist.";

        case EFI_TIMEOUT:
        return "The timeout time expired.";

        case EFI_NOT_STARTED:
        return "The protocol has not been started.";

        case EFI_ALREADY_STARTED:
        return "The protocol has already been started.";

        case EFI_ABORTED:
        return "The operation was aborted.";

        case EFI_ICMP_ERROR:
        return "An ICMP error occurred during the network operation.";

        case EFI_TFTP_ERROR:
        return "A TFTP error occurred during the network operation.";

        case EFI_PROTOCOL_ERROR:
        return "A protocol error occurred during the network operation.";

        case EFI_INCOMPATIBLE_VERSION:
        return "The function encountered an internal version that was incompatible with a version requested by the caller.";

        case EFI_SECURITY_VIOLATION:
        return "The function was not performed due to a security violation.";

        case EFI_CRC_ERROR:
        return "A CRC error was detected.";

        case EFI_END_OF_MEDIA:
        return "Beginning or end of media was reached.";

        case EFI_END_OF_FILE:
        return "The end of the file was reached";

        case EFI_INVALID_LANGUAGE:
        return "The language specified was invalid.";

        case EFI_COMPROMISED_DATA:
        return "The security status of the data is unknown or compromised and the data must be updated or replaced to restore a valid security status.";

        default:
        return "Unknown error.";
    }
}
