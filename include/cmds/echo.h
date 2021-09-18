#ifndef ECHO_H
#define ECHO_H
#include <uefi.h>

inline void EchoCmd(char args[])
{
    if (args) printf("\n%s", args);
}

inline const char* EchoBrief(void)
{
    return "Print a string. Used for debugging.";
}

inline const char* EchoLong(void)
{
    return "Usage: echo <string>";
}
#endif