#include "cmds/mkdir.h"

void MkdirCmd(char args[], char** currPathPtr)
{
    if (args == NULL)
    {
        printf("\nmkdir: no directory name specified");
        return;
    }

    boolean_t isDynamicMemory = FALSE;
    char* path = MakeFullPath(args, *currPathPtr, &isDynamicMemory);
    if (path == NULL)
    {
        printf("\nmkdir: no directory name specified");
        return;
    }

    DIR* dir = opendir(path);
    if (dir != NULL)
    {
        closedir(dir);
        printf("\nmkdir: directory already exists");
    }
    else
    {
        // Creates a new directory and frees the pointer to it
        FILE* fp = fopen(path, "wd");
        if (fp != NULL)
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
    if (isDynamicMemory) BS->FreePool(path);
}

const char* MkdirBrief(void)
{
    return "Create a directory.";
}

const char* MkdirLong(void)
{
    return "Usage: mkdir <path or dirname>";
}
