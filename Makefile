ARCH = x86_64
TARGET = lucidloader_x64.efi
SRCS = $(wildcard src/*.c) $(wildcard src/cmds/*.c) $(wildcard src/protocols/*.c)
CFLAGS = -Iinclude -Wall -Wextra -pedantic -Wno-unused-parameter -O2

include uefi/Makefile
