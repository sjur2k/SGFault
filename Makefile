CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = src/sgfault.c src/utils.c src/lexer.c src/compiler_args.c

COMPILER_BINARY = build/sgfault
FILE = testing/test.sg
NAME = $(basename $(notdir $(FILE)))

.PHONY: mkdir build install clean run

mkdir:
	@mkdir -p build

build: mkdir
	@$(CC) $(CFLAGS) $(SRC) -o $(COMPILER_BINARY)

run: build
	@echo "Building..."
	@$(COMPILER_BINARY) $(FILE)
	@echo "\nRunning..."
	@./$(NAME)

install: build
	@cp build/sgfault /usr/local/bin/sgfault
	@rm -rf build/
	@echo "Installed SGfault\n"

clean:
	@rm -rf build/

	