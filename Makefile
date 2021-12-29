TARGET = ezboot_x64.efi
SRCS = $(wildcard src/*.c) $(wildcard src/cmds/*.c)
CFLAGS = -Iinclude -Wall -Wextra -pedantic -Wno-unused-parameter

include uefi/Makefile
