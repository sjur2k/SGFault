CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -g
SRC = src/sgfault.c src/utils.c src/lexer.c src/compiler_args.c src/parser.c

COMPILER_BINARY = build/sgfault
FILE = testing/test.sg
NAME = $(basename $(notdir $(FILE)))

.PHONY: mkdir build install clean run

mkdir:
	@mkdir -p build

build: mkdir
	@$(CC) $(CFLAGS) $(SRC) -o $(COMPILER_BINARY)

run: build
	@echo "\n\033[32mBuilding...\033[0m"
	@$(COMPILER_BINARY) $(FILE) -o $(dir $(FILE))$(NAME)
	@echo "\n\033[32mRunning...\033[0m"
	@./$(dir $(FILE))$(NAME)

install: build
	@cp build/sgfault /usr/local/bin/sgfault
	@rm -rf build/
	@echo "Installed SGfault\n"

clean:
	@rm -rf build/
	@rm -f testing/test

	