#include "logger.h"
#include "bootmenu.h"
#include "screen.h"

int main(int argc, char** argv)
{
    if(!InitLogger())
    {
        printf("Failed to initialize logger. Logging disabled.\n");
    }

    // Try to set max console size and store the size in global variables
    if (!SetMaxConsoleSize())
    {
        if (!QueryCurrentConsoleSize())
        {
            Log(LL_WARNING, 0, "Falling back to the ClearScreen method to redraw the screen.");
        }
    }

    StartBootManager();

    // This should never be reached
    return 1;
}
