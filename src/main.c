#include <uefi.h>
#include "logger.h"
#include "bootmenu.h"

int main(int argc, char** argv)
{
    if(!InitLogger())
    {
        printf("Failed to initialize logger. Logging disabled.\n");
    }

    MainMenu();

    // This should never be reached
    return 1;
}
