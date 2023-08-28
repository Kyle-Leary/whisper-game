# Variables
CC := gcc

# there are some cases where we literally can't align a vec4 on a 16 byte boundary.
# skip the CGLM check. https://cglm.readthedocs.io/en/stable/opt.html
CFLAGS += -Wall -DCGLM_ALL_UNALIGNED=1 -DDEBUG=1

INCLUDES += -I. -Isrc -Ideps/wjson/api -Ideps -Ideps/stb -Ideps/libwhisper/api

# ignore the whole backend folder, and only add in the backend subfolder.
ALL_BACKEND := src/backends

# The final target (change this to your desired target name)
TARGET:=whisper

CFLAGS += 
# modify the include variables AFTER config.mk.
LIBS := -lGLEW -lGL -lglfw -lopenal -lalut -lm -lpthread -g

BACKEND_DIR := linux

TEST_TARGET := whisper_test

# Directories
SRC_DIR := src 

WJSON := deps/wjson/libwjson.a
LIBWHISPER := deps/libwhisper/libwhisper.a
GLPP := deps/glpp/glpp

# Get a list of all .c files
SRCS := $(shell find $(SRC_DIR) -name '*.c' -type f)
# Filter out .test.c files from the list
SRCS := $(filter-out %.test.c, $(SRCS))

# Object files (corresponding .o files in the obj/ directory)
OBJS := $(patsubst %.c,%.o,$(SRCS))
SYMBOLS := $(OBJS)
SYMBOLS += $(WJSON)
SYMBOLS += $(LIBWHISPER)

REQUIREMENTS := $(SYMBOLS)

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
SHADERS := $(filter-out %.vs,$(SHADERS))
SHADERS := $(filter-out %.fs,$(SHADERS))
# just in case, even though we're already patsubst'ing it.
SHADERS := $(filter-out %.shader,$(SHADERS))

REQUIREMENTS += $(SHADERS)
REQUIREMENTS += $(GLPP)

TEST_REQUIREMENTS += $(TEST_SYMBOLS)
TEST_REQUIREMENTS += $(REQUIREMENTS)

$(info OBJS is $(OBJS))

all: $(TARGET)
	@echo "Target built at path ./$(TARGET)."

test: $(TEST_TARGET)
	@echo "Test built at path ./$(TEST_TARGET). Running..."
	./$(TEST_TARGET)

$(WJSON):
	make -C deps/wjson
$(LIBWHISPER):
	make -C deps/libwhisper
$(GLPP):
	make -C deps/glpp

# for both, just compile all the symbols into TARGET with CC
$(TARGET): $(REQUIREMENTS) main.o 
	$(CC) -o $@ $(SYMBOLS) main.o $(CFLAGS) $(LIBS)

$(TEST_TARGET): $(REQUIREMENTS) $(TEST_REQUIREMENTS) testmain.o
	$(CC) -o $@ $(TEST_SYMBOLS) testmain.o $(CFLAGS) $(LIBS)

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
	rm -f $(shell find . -name "*.o") $(TARGET) $(TEST_TARGET) $(WJSON) $(LIBWHISPER) $(GLPP) $(SHADERS)

# always rebuild test.c
# always rebuild shaders, it's cheap.
.PHONY: clean testmain.c %.shader.pp
