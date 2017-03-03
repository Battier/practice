CC       ?= gcc
CFLAGS   += -Wno-visibility -Wno-incompatible-pointer-types -Wall -Wextra
CFLAGS   += -O0 -g
CFLAGS   += -std=c99
LDFLAGS  += -lreadline

PROGRAM := cli
C_FILES := $(wildcard src/*.c)

.PHONY: all clean

all: 
	$(CC) $(CFLAGS) $(C_FILES) -o $(PROGRAM) $(LDFLAGS)

clean:
	rm -f $(PROGRAM)
