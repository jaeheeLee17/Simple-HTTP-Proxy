CC=gcc
CFLAGS=-g -o
TARGETS=myproxy
all: $(TARGETS)
.PHONY: all
%:
	$(CC) $(CFLAGS) $@ $@.c
clean:
	rm $(TARGETS)

