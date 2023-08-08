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

SRCS := $(shell find $(SRC_DIR) -type f -name "*.c" | grep -v "$(ALL_BACKEND)") 
SRCS += $(shell find $(ALL_BACKEND)/$(BACKEND_DIR) -type f -name "*.c")

# Object files (corresponding .o files in the obj/ directory)
OBJS := $(patsubst %.c,%.o,$(SRCS))
OBJS += $(WJSON)

$(info OBJS is $(OBJS))

all: $(TARGET)
	@echo "Target built at path ./$(TARGET)."

test: $(TEST_TARGET)
	@echo "Test built at path ./$(TEST_TARGET). Running..."
	./$(TEST_TARGET)

$(WJSON):
	make -C deps/wjson

# compile different definitions of main into the binary depending on what we're building.
$(TARGET): $(OBJS) main.o 
	@echo $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

$(TEST_TARGET): $(OBJS) test.o
	$(CC) -o $@ $^ $(LIBS)

# Pattern rule to compile .c files into .o files
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES) -g

clean:
	rm -f $(shell find . -name "*.o") $(TARGET) $(TEST_TARGET) $(WJSON)

.PHONY: clean
