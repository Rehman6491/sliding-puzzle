generate:
	@echo "Building project..."
	gcc src/main.c -std=c99 -W -Wall -pedantic -lm -lncurses
