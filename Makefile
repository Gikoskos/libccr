ifndef CC
    CC = gcc
else
    ifeq($(CC),)
        CC = gcc
    endif
endif
    
CFLAGS = -pthread -g -Wall -Wextra -pedantic -std=c99 -I./
LINKER = -L./ -l$(LIB_NAME)

TEST_EXES = test_macro test_lib
LIB_NAME = ccr

LIB_SRC = $(LIB_NAME:=.c)

LIB_OBJ = $(LIB_SRC:.c=.o)
LIB_AR = $(patsubst %, lib%.a, $(LIB_NAME))


.PHONY: clean all

all: $(LIB_AR) $(TEST_EXES)

$(LIB_AR): $(LIB_OBJ)
	ar rcs $@ $^

$(LIB_OBJ): $(LIB_SRC)
	$(CC) $(CFLAGS) -c $^

$(TEST_EXES): % : %.c
	$(CC) $(CFLAGS) $< -o $@ $(LINKER)

clean:
	@rm -f $(LIB_OBJ) $(TEST_EXES) $(LIB_AR)
