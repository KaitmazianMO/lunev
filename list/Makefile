CC        := gcc
CCFLAGS   = -Wall -Wextra -Werror $($(@F)_FLAGS)
test.o_FLAGS = -Wno-unused-variable -Wno-unused-value -Wno-unused-parameter

BUILD_DIR := build
MAKE_STUFF_DIR  := $(BUILD_DIR)/makestuff/
PATH_TO_LIB := $(LIB_INSTALL_DIR)/$(LIB_NAME)

LIB_SOURCES  := $(wildcard list/*.c)
TEST_SOURCES := $(wildcard test/*.c)
ALL_SOURCES  := $(LIB_SOURCES) $(TEST_SOURCES)
ALL_OBJECTS  := $(ALL_SOURCES:.c=.o)

LIB_NAME := list.a
LIB_INSTALL_DIR := ${BUILD_DIR}/lib

$(BUILD_DIR)/run_test: $(addprefix $(BUILD_DIR)/, $(TEST_SOURCES:.c=.o)) $(PATH_TO_LIB)
	$(CC) $^ $(CCFLAGS) -o $@ 

$(PATH_TO_LIB): $(addprefix $(BUILD_DIR)/, $(LIB_SOURCES:.c=.o))
	mkdir -p $(@D)
	ar rcs $@ $^

$(addprefix $(BUILD_DIR)/, $(ALL_OBJECTS)): $(BUILD_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -MMD -MF$(subst .o,.d, $@) -MT$@ $(CCFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)