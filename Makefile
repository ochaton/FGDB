CC = cc -std=c11
CFLAGS += -Wall -Wformat-security -Wignored-qualifiers -Winit-self \
		-Wswitch-default -Wfloat-equal -Wshadow -Wpointer-arith \
		-Wtype-limits -Wempty-body \
		-Wmissing-field-initializers -Wextra \
		-Wno-pointer-to-int-cast -Wno-int-conversion  \

TARGET = tcp-ev-server
# Directories with source code
SRC_DIR = src
INCLUDE_DIR = include

BUILD_DIR = build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin

# Link libraries gcc flag: library will be searched with prefix "lib".
LDFLAGS = -lev

# Add headers dirs to gcc search path
CFLAGS += -I $(INCLUDE_DIR)

# Helper macros
# subst is sensitive to leading spaces in arguments.
make_path = $(addsuffix $(1), $(basename $(subst $(2), $(3), $(4))))
# Takes path list with source files and returns pathes to related objects.
src_to_obj = $(call make_path,.o, $(SRC_DIR), $(OBJ_DIR), $(1))
# Takes path list with object files and returns pathes to related dep. file.

# All source files in our project that must be built into movable object code.
CFILES := $(wildcard $(SRC_DIR)/*.c)
OBJFILES := $(call src_to_obj, $(CFILES))

# Default target (make without specified target).
.DEFAULT_GOAL := all

# Alias to make all targets.
.PHONY: all
all: release

release: CC += -O2
release: CFLAGS += -fPIC -Werror
release: directories $(BIN_DIR)/$(TARGET)

debug: CC += -DDEBUG -ggdb3
debug: directories $(BIN_DIR)/$(TARGET)

directories:
	mkdir -p $(BUILD_DIR) $(OBJ_DIR) $(BIN_DIR)

# Suppress makefile rebuilding.
Makefile: ;

# Rules for compiling targets
$(BIN_DIR)/$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) $(filter %.o, $^) -o $@ $(LDFLAGS)

# Pattern for generating dependency description files (*.d)
$(DEP_DIR)/%.d: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -E -MM -MT $(call src_to_obj, $<) -MT $@ -MF $@ $<

# Pattern for compiling object files (*.o)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $(call src_to_obj, $<) $<

# Fictive target
.PHONY: clean
# Delete all temprorary and binary files
clean:
	rm -rf $(BUILD_DIR)

# This section for runing ad testing

# If the first argument is "run"...
ifeq (run,$(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "run"
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  # ...and turn them into do-nothing targets
  $(eval $(RUN_ARGS):;@:)
endif

run: $(BIN_DIR)/$(TARGET)
	$^ $(RUN_ARGS)

ifneq ("$(wildcard test/)", "")
	TEST_DIR = "test"
endif

ifdef TEST_DIR
test: $(BIN_DIR)/$(TARGET)
	make -C $(TEST_DIR) run
endif
