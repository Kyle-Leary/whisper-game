# Variables
CC := gcc
WINDOWS_CC := x86_64-w64-mingw32-gcc

# The final target (change this to your desired target name)
TARGET := whisper
WINDOWS_TARGET := $(TARGET).exe
TARGETS := $(TARGET) $(WINDOWS_TARGET)

# there are some cases where we literally can't align a vec4 on a 16 byte boundary.
# skip the CGLM check. https://cglm.readthedocs.io/en/stable/opt.html
CFLAGS := -DCGLM_ALL_UNALIGNED=1 -DDEBUG=1 -DAREA_HOT_RELOAD=1
CFLAGS += -Wall -Wno-missing-braces -Wno-unused-variable -rdynamic

LIBS := -lGLEW -lGL -lswscale -lavutil -lavformat -lavcodec -lz -lglfw -laltrace_record -lm -lpthread -g
INCLUDES += -I. -Isrc -Ideps/wjson/api -Ideps -Ideps/stb -Ideps/libwhisper/api

TEST_TARGET := whisper_test

# Directories
SRC_DIR := src 
AREA_DIR := areas 
BUILD_DIR := dist

WJSON := deps/wjson/libwjson.a
LIBWHISPER := deps/libwhisper/libwhisper.a
GLFW := deps/glfw/libglfw3.a

# all the static libaries we're linking against.
STATIC := $(WJSON) $(LIBWHISPER) $(GLFW)

GLPP := deps/glpp/glpp

# the rules to make these deps will be automatically generated, simply use the deps
# where you need and it should work.
DEPS := $(GLPP) $(STATIC)

AREA_DIR := $(strip areas)
# Get a list of all .c files
AREA_SRCS := $(shell find $(AREA_DIR) -name '*.c' -type f)
AREA_LIBS := $(patsubst $(AREA_DIR)/%.c,$(BUILD_DIR)/$(AREA_DIR)/libarea%.so,$(AREA_SRCS))

# Get a list of all .c files
SRCS := $(shell find $(SRC_DIR) -name '*.c' -type f)
# Filter out .test.c files from the list
SRCS := $(filter-out %.test.c, $(SRCS))

# Object files (corresponding .o files in the obj/ directory)
OBJS := $(patsubst %.c,%.o,$(SRCS))
SYMBOLS := $(OBJS)
SYMBOLS += $(STATIC)

# don't directly link the area libraries into the executable, we're using them at runtime.
# we still need them, though.
REQUIREMENTS := $(SYMBOLS) $(AREA_LIBS)

TEST_SRCS := $(shell find $(SRC_DIR) -name '*.test.c' -type f)
TEST_OBJS := $(patsubst %.test.c,%.test.o,$(TEST_SRCS))
TEST_SYMBOLS := $(TEST_OBJS)
# it has all the symbols as the normal build, just some extras for the test compilation units.
TEST_SYMBOLS += $(SYMBOLS)

SHADER_PATH := assets/shaders
SHADERS := $(shell find $(SHADER_PATH) -type f)

# target preprocessed shaders.
SHADERS := $(patsubst %.shader,%.shader.pp,$(SHADERS))

# don't accidentally delete these in the make clean.
SHADERS := $(filter-out %.glinc,$(SHADERS))
# just in case, even though we're already patsubst'ing it.
SHADERS := $(filter-out %.shader,$(SHADERS))

# extra requirements that aren't necessarily object/symbol files.
REQUIREMENTS += $(SHADERS)
REQUIREMENTS += $(GLPP)

TEST_REQUIREMENTS += $(TEST_SYMBOLS)
TEST_REQUIREMENTS += $(REQUIREMENTS)

# use linux build by default.
all: setup $(TARGET)
windows: setup $(WINDOWS_TARGET)

# just build the area libraries.
areas: setup $(AREA_LIBS)

# doing an auto submodule init here was a BAD IDEA. removed it.
setup: 
	mkdir -p $(BUILD_DIR)/$(AREA_DIR)
	rm -f *.altrace

$(TARGET): $(REQUIREMENTS) main.o 
	@echo "Making target $(1)"
	$(CC) -o $@ $(SYMBOLS) main.o $(CFLAGS) $(LIBS)

$(WINDOWS_TARGET): $(REQUIREMENTS) main.o 
	@echo "Making target $(1)"
	$(WINDOWS_CC) -o $@ $(SYMBOLS) main.o $(CFLAGS) $(LIBS)

test: $(TEST_TARGET)
	@echo "Test built at path ./$(TEST_TARGET). Running..."
	./$(TEST_TARGET)

# just make each dependency using a generic make -C in the $(dir $(DEP)) base dir
# of the dependency.
define create_rule
$(1):
	@echo "Making dependency in $(dir $(1))"
	make -C $(dir $(1))
endef

# eval to dynamically generate the rule
# call to call a user-defined function.
# in total, this defines all the rules we need dynamically, each time the makefile is run.
$(foreach dep,$(DEPS),$(eval $(call create_rule,$(dep))))


$(TEST_TARGET): $(REQUIREMENTS) $(TEST_REQUIREMENTS) testmain.o
	$(CC) -o $@ $(TEST_SYMBOLS) testmain.o $(CFLAGS) $(LIBS)

$(BUILD_DIR)/$(AREA_DIR)/libarea%.so: $(AREA_DIR)/%.c
	$(CC) -shared -o $@ $< $(CFLAGS) $(INCLUDES) -g -fPIC

# Pattern rule to compile .c files into .o files
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES) -g

# the entire thing is handled by this generator script, just compile this and it should work
# as the test main. it calls all the testing void functions.
testmain.c:
	./extract_test_functions.sh > $@

# with the -I, glpp will automatically pick up on any shader includes
# in the $(SHADER_PATH) directory.
%.shader.pp: %.shader $(GLPP)
	$(GLPP) $< -o $@ -I$(SHADER_PATH)

clean:
	rm -f $(REQUIREMENTS) $(TARGETS) $(TEST_TARGET) 

# always rebuild test.c
# always rebuild shaders, it's cheap.
.PHONY: clean testmain.c %.shader.pp all al-debug
.SECONDARY: $(OBJS)
.PRECIOUS: %.o %.c
