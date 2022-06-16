TARGET := calc

SOURCE_FILES = main.c integral.c static_thread.c cpu_conf.c
OBJECT_FILES = $(SOURCE_FILES:.c=.o)

CFLAGS = -Wall -Werror -g -O0 -MMD -DMY_DEBUG
DYN_LIBS = -lm -pthread

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECT_FILES)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(DYN_LIBS)

-include $(wildcard *.d)

clean:
	rm -f *.o *.d $(TARGET)