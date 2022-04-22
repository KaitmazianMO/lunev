TARGET = calc
INCLUDE_FILES = debug.h calc_int.h cpu_conf.h
SOURCE_FILES = main.c calc_int.c cpu_conf.c
OBJECT_FILES = $(SOURCE_FILES:.c=.o)

CFLAGS = -Wall -Werror -g -O0
DYN_LIBS = -lm -pthread

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECT_FILES) $(INCLUDE_FILES)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(DYN_LIBS)

clean:
	rm $(OBJECT_FILES) $(TARGET)