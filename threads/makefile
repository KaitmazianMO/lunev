TARGET = calc
HT_RAGET = ht_calc
HT_FLAG = HT
INCLUDE_FILES = debug.h calc_int.h cpu_conf.h
SOURCE_FILES = main.c calc_int.c cpu_conf.c
OBJECT_FILES = $(SOURCE_FILES:.c=.o)

CFLAGS = -Wall -Werror -g -O0 -DMY_DEBUG
DYN_LIBS = -lm -pthread

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECT_FILES) $(INCLUDE_FILES)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(DYN_LIBS)

$(HT_RAGET): $(OBJECT_FILES) $(INCLUDE_FILES)
	$(CC) $(CFLAGS) -D$(HT_FLAG) $^ -o $(TARGET) $(DYN_LIBS)

clean:
	rm -f $(OBJECT_FILES) $(TARGET)