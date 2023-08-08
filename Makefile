# Variables
CC := gcc
CFLAGS += 

include config.mk

BACKEND_DIR := linux

# Directories
SRC_DIR := src 
OBJ_DIR := .

SRCS := $(shell find $(SRC_DIR) -type f -name "*.c" | grep -v "$(ALL_BACKEND)") 
SRCS += $(shell find $(ALL_BACKEND)/$(BACKEND_DIR) -type f -name "*.c")

# Object files (corresponding .o files in the obj/ directory)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))


$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(INCLUDES) -lGLEW -lGL -lglfw -lopenal -lalut -lm -g

# Pattern rule to compile .c files into .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -g

clean:
	rm -f $(shell find $(OBJ_DIR) -name "*.o") $(TARGET)
