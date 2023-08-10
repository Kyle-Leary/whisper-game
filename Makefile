# Variables
CC := gcc

include config.mk

CFLAGS += 
# modify the include variables AFTER config.mk.
LIBS := -lGLEW -lGL -lglfw -lopenal -lalut -lm -g

BACKEND_DIR := linux

TEST_TARGET := whisper_test

# Directories
SRC_DIR := src 

WJSON := deps/wjson/libwjson.a
LIBWHISPER := deps/libwhisper/libwhisper.a
GLPP := deps/glpp/glpp

SRCS := $(shell find $(SRC_DIR) -type f -name "*.c" | grep -v "$(ALL_BACKEND)") 
SRCS += $(shell find $(ALL_BACKEND)/$(BACKEND_DIR) -type f -name "*.c")
# then, include all the general backend function implementations.
SRCS += $(shell find $(ALL_BACKEND)/general -type f -name "*.c")

# Object files (corresponding .o files in the obj/ directory)
OBJS := $(patsubst %.c,%.o,$(SRCS))
SYMBOLS := $(OBJS)
SYMBOLS += $(WJSON)
SYMBOLS += $(LIBWHISPER)

REQUIREMENTS := $(SYMBOLS)

SHADER_PATH := assets/shaders
SHADERS := $(shell find $(SHADER_PATH) -type f)
# target preprocessed shaders.
SHADERS := $(patsubst %.vs,%.vs.pp,$(SHADERS))
SHADERS := $(patsubst %.fs,%.fs.pp,$(SHADERS))
# don't delete the common includes in make clean
SHADERS := $(filter-out %.glinc,$(SHADERS))

REQUIREMENTS += $(SHADERS)
REQUIREMENTS += $(GLPP)

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

# compile different definitions of main into the binary depending on what we're building.
$(TARGET): $(REQUIREMENTS) main.o 
	$(CC) -o $@ $(SYMBOLS) main.o $(LIBS)

$(TEST_TARGET): $(REQUIREMENTS) test.o
	$(CC) -o $@ $(SYMBOLS) test.o $(LIBS)

# Pattern rule to compile .c files into .o files
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES) -g

# note: right now, only vs and fs files are being directly glpp'd. this means that
# the common includes themselves may not, so watch out for that.
# with the -I, it'll automatically pick up on any shader includes in the $(SHADER_PATH)
# directory.
%.vs.pp: %.vs $(GLPP)
	$(GLPP) $< -o $@ -I$(SHADER_PATH)
%.fs.pp: %.fs $(GLPP)
	$(GLPP) $< -o $@ -I$(SHADER_PATH)

clean:
	rm -f $(shell find . -name "*.o") $(TARGET) $(TEST_TARGET) $(WJSON) $(LIBWHISPER) $(GLPP) $(SHADERS)

.PHONY: clean
