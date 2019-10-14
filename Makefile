generate:
	@echo "Building project..."
	gcc src/main.c -std=c99 -W -Wall -Werror -Wextra -pedantic -lm -lncurses
