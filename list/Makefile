CC        := gcc
CCFLAGS   = -Wall -Wextra -Wpedantic -Werror
DBG_FLAGS := -g -O0 --coverage

BUILD_DIR := build
LIB_DIR   := $(BUILD_DIR)/lib
OBJ_DIR   := $(BUILD_DIR)/obj
GCOV_DIR  := $(BUILD_DIR)/gcov
ALL_DIRS  := $(BUILD_DIR) $(LIB_DIR) $(OBJ_DIR) $(GCOV_DIR) 

LIB_SRC   := $wildcard(lib/*.c)  

all: dirs

dirs: $(ALL_DIRS)
	mkdir -p $(ALL_DIRS)

$(OBJ_DIR)/list.a: $(LIB_DIR)/list.o
	ar r $(LIB_DIR)/list.o --output $(LIB_DIR)

clean:
	rm -rf 