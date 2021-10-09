TARGET = bootx64.efi
SRCS = $(wildcard src/*.c) $(wildcard src/cmds/*.c)
CFLAGS = -Iinclude #-Wall -Wextra -pedantic

include uefi/Makefile
