CC      = gcc
CFLAGS += -std=c99 -Wall -Werror -MMD

all: server client

server: server.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

THREAD_SRC = $(wildcard threads/*.c)
client: client.o $(THREAD_SRC:.c=.o)
	$(CC) $(CFLAGS) $^ -o $@ -lm -pthread

-include $(wildcard *.d)

.PHONY: clean
clean:
	rm -f server client *.o threads/*.o *.d threads/*.d