generate:
        @echo "Creating empty text files..."
        gcc main.c -std=C99 -W -Wall -Werror -Wextre -pedantic -lm -lncurses

clean:
        @echo "Cleaning up..."
        rm *.txt
