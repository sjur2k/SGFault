CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = src/sgfault.c

NAME = $(basename $(notdir $(FILE)))
COMPILER_BINARY = build/sgfault


.PHONY: build install clean mkdir

mkdir:
	@mkdir -p build

build: mkdir
	@$(CC) $(CFLAGS) $(SRC) -o $(COMPILER_BINARY)

install: build
	@cp build/sgfault /usr/local/bin/sgfault
	@rm -rf build/
	@echo "Installed SGfault\n"

clean:
	@rm -rf build/

	