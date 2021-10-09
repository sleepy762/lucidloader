#include "cmds/mkdir.h"

void MkdirCmd(char args[], char** currPathPtr)
{
    if (!args)
    {
        printf("\nmkdir: no directory name specified");
        return;
    }

    boolean_t dynMemFlag = FALSE;
    char* path = MakeFullPath(args, *currPathPtr, &dynMemFlag);
    if (path == NULL)
    {
        printf("\nmkdir: no directory name specified");
        return;
    }

    DIR* dir;
    if ((dir = opendir(path)))
    {
        closedir(dir);
        printf("\nmkdir: directory already exists");
    }
    else
    {
        FILE* fp = fopen(path, "wd");
        if (fp)
        {
            fclose(fp);
        }
        else
        {
            if (errno == EROFS)
            {
                printf("\nmkdir: permission denied - readonly filesystem");
            }
            else
            {
                printf("\nmkdir: failed to create directory");
            }
        }
    }
    if (dynMemFlag) BS->FreePool(path);
}

const char* MkdirBrief(void)
{
    return "Create a directory.";
}

const char* MkdirLong(void)
{
    return "Usage: mkdir <path>";
}
