CC = gcc
CFLAGS = -pthread -g -Wall -Wextra -pedantic -std=c99 -I../../
LINKER =

SRC = $(wildcard *.c)

EXE = bridge

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LINKER)

.PHONY: clean

clean:
	@rm -f $(EXE)
