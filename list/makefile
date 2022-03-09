CC        := gcc
CCFLAGS   = -Wall -Wextra -Werror --coverage $($(@F)_FLAGS)
test.o_FLAGS = -Wno-unused-variable -Wno-unused-value -Wno-unused-parameter

TARGET := run_tests

PROJECT_DIR   := $(shell pwd)
BUILD_DIR     := $(PROJECT_DIR)/build
DIRS_TO_BUILD  = list test
TARGET        := $(addprefix $(BUILD_DIR)/, $(TARGET))   

SOURCE_DIRS := $(foreach dir, $(DIRS_TO_BUILD), $(addprefix $(PROJECT_DIR)/, $(dir)))
OBJECT_DIRS := $(foreach dir, $(DIRS_TO_BUILD), $(addprefix $(BUILD_DIR)/, $(dir)))

SOURCE_FILES        := $(foreach dir, $(SOURCE_DIRS), $(wildcard $(dir)/*.c))
OBJECT_FILES        := $(subst $(PROJECT_DIR),$(BUILD_DIR),$(SOURCE_FILES:.c=.o))
INCLUDE_FILES       := $(SOURCE_FILES:.c=.h)
GCOV_COVERAGE_FILES := $(OBJECT_FILES:.o=.gcno)
DEPENDENCIES        := $(OBJECT_FILES:.o=.d)

.PHONY: clean test coverage valgrind

all: $(TARGET)

test: $(TARGET)
	@$(TARGET)

coverage: test
	@mkdir -p $(BUILD_DIR)/gcov && cd $(BUILD_DIR)/gcov && \
	gcov $(GCOV_COVERAGE_FILES)

valgrind: $(TARGET)
	@valgrind --leak-check=full $(TARGET)

$(TARGET): $(OBJECT_FILES)
	$(CC) $^ $(CCFLAGS) -o $@ 

$(OBJECT_FILES): $(BUILD_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -MD -MF $(@:.o=.d) -MT $@ $(CCFLAGS) -c $< -o $@

clear_dependensies:
	find -name *.d | xargs rm -f

$(DEPENDENCIES): $(OBJECT_FILES)

include $(DEPENDENCIES)

clean:
	rm -rf $(BUILD_DIR)