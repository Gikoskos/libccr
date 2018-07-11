CC = gcc
CFLAGS = -pthread -g -Wall -Wextra -pedantic -std=c99 -I../4
LINKER =

SRC = $(wildcard *.c)

EXE = rollercoaster

$(EXE): $(SRC) common.h
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LINKER)

.PHONY: clean

clean:
	@rm -f $(EXE)
