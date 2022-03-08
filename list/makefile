CC        := gcc
CCFLAGS   = -Wall -Wextra -Werror $($(@F)_FLAGS)
test.o_FLAGS = -Wno-unused-variable -Wno-unused-value -Wno-unused-parameter

TARGET := run_tests

PROJECT_DIR   := $(shell pwd)
BUILD_DIR     := $(PROJECT_DIR)/build
MAKE_DIR      := $(BUILD_DIR)/makestuff
DIRS_TO_BUILD = list test

SOURCE_DIRS := $(foreach dir, $(DIRS_TO_BUILD), $(addprefix $(PROJECT_DIR)/, $(dir)))
OBJECT_DIRS := $(foreach dir, $(DIRS_TO_BUILD), $(addprefix $(BUILD_DIR)/, $(dir)))

SOURCE_FILES  := $(foreach dir, $(SOURCE_DIRS), $(wildcard $(dir)/*.c))
OBJECT_FILES  := $(subst $(PROJECT_DIR),$(BUILD_DIR),$(SOURCE_FILES:.c=.o))
INCLUDE_FILES := $(SOURCE_FILES:.c=.h)
DEPENDENCIES  := $(OBJECT_FILES:.o=.d)

all: directories dbg $(TARGET)

dbg:
	echo $(SOURCE_DIRS)
	echo $(OBJECT_DIRS)
	echo $(SOURCE_FILES)
	echo $(OBJECT_FILES)
	echo $(INCLUDE_FILES)
	echo $(DEPENDENCIES) 

$(TARGET): $(OBJECT_FILES)
	$(CC) $^ $(CCFLAGS) -o $@ 

$(OBJECT_FILES): $(BUILD_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -MMD -MF$($@:.o=.d) -MT$@ $(CCFLAGS) -c $< -o $@

directories:
	mkdir -p $(BUILD_DIR) $(MAKE_DIR)
	cd $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)