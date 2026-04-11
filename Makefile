CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wstrict-prototypes -g -DDATA_DIR=\"/usr/local/share/sgfault\"
SRC = src/main.c src/utils.c src/lexer.c src/compiler_args.c src/parser.c

COMPILER_BINARY = build/sgfault
FILE = testing/test.sg
NAME = $(basename $(notdir $(FILE)))

.PHONY: mkdir build install clean run

mkdir:
	@mkdir -p build

build: mkdir
	@echo "\n\033[32mBuilding...\033[0m"
	@$(CC) $(CFLAGS) -o $(COMPILER_BINARY) $(SRC) 

run: build # ONLY FOR TESTING
	@$(COMPILER_BINARY) -o $(dir $(FILE))$(NAME) $(FILE)
	@echo "\n\033[32mRunning compiled file...\033[0m"
	@./$(dir $(FILE))$(NAME)

install: mkdir
	@echo "\n\033[32mInstalling SGFault ...\033[0m"
	@$(CC) $(CFLAGS) -o $(COMPILER_BINARY) $(SRC) 
	@cp build/sgfault /usr/local/bin/sgfault
	@mkdir -p /usr/local/share/sgfault/docs
	@cp docs/* /usr/local/share/sgfault/docs
	@echo "\n\033[32mInstallation sucessful!\n\033[0m"

clean:
	@rm -rf build/
	@rm -f testing/test

uninstall:
	@echo "\n\033[32mUninstalling SGFault ...\033[0m"
	@rm -rf /usr/local/share/sgfault
	@rm -f /usr/local/bin/sgfault
	@echo "\n\033[32mUninstallation sucessful!\n\033[0m"