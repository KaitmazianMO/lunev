CC        := gcc
CCFLAGS   = -Wall -Wextra -Wpedantic -Werror
DBG_FLAGS := -g -O0 --coverage -MD 

BUILD_DIR := build
LIB_DIR   := $(BUILD_DIR)/lib
OBJ_DIR   := $(BUILD_DIR)/obj
TEST_DIR  := $(BUILD_DIR)/test
GCOV_DIR  := $(TEST_DIR)/gcov
ALL_DIRS  := $(LIB_DIR) $(OBJ_DIR) $(TEST_DIR) $(GCOV_DIR)

LIB_SRC   := $(wildcard src/*.c)  
TEST_SRC  := $(wildcard test/*.c)

.PHONY clean

all: dirs test

dirs: $(ALL_DIRS)

$(ALL_DIRS):
	mkdir -p $@

test: $(TEST_SRC) $(LIB_DIR)/list.a
	$(CC) $(CCFLAGS) $^ -o $(TEST_DIR)/$@

$(LIB_DIR)/%.a: $(OBJ_DIR)/%.o
	ar rcs $@ $^

$(OBJ_DIR)/%.o: %.c
	$(CC) -c $(CCFLAGS) $^ $(OBJ_DIR)/$@

clean:
	rm -rf $(BUILD_DIR)