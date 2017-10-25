# Makefile template for shared library
CFLAGS := -fPIC -O3 -g -Wall -Werror
CC := gcc
NAME := log

#SOURCE := $(wildcard *.c)
SOURCE := log.c
OBJECT := $(SOURCE:.c=.o)

LIBSO  = lib$(NAME).so
LIBA   = lib$(NAME).a

test: clean $(NAME)_test
	LD_LIBRARY_PATH=. ./$(NAME)_test

$(NAME)_test: lib$(NAME).so
	$(CC) $(NAME)_test.c -o $@ -L. -l$(NAME)

lib: $(LIBSO) $(LIBA)

$(LIBSO): $(OBJECT)
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(LIBSO) -o $(LIBSO) $(OBJECT)

$(LIBA): $(OBJECT)
	$(AR) $(ARFLAGS) $(LIBA) $(OBJECT)

clean:
	$(RM) $(NAME)_test *.o *.so* *.a
