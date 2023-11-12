CC=gcc
CFLAGS=-g -D__USE_FIXED_PROTOTYPES__ -ansi -std=gnu99

SRC = src
SRCS = $(wildcard $(SRC)/*.c)

main: 
	$(CC) $(CFLAGS) -o repl $(SRCS) && ./repl
