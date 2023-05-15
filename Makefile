# Copyright (c) 2023, Robert-Ioan Constantinescu

# Directories
SRC_DIR   := src
BUILD_DIR := build
BIN_DIR   := bin

# Files
TARGET  := client
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

# Compiler & Flags
CC     := gcc
CFLAGS := -Wall -Wextra -g

# Default target
all: $(BIN_DIR)/$(TARGET)

# Create build and bin directories if they don't exist
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Compile source files into objects
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link objects to create the target
$(BIN_DIR)/$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Run rule
run: $(BIN_DIR)/$(TARGET)
	@./$^

# Clean rule
clean:
	rm -rf $(TARGET).zip $(BUILD_DIR) $(BIN_DIR)

# Pack rule
pack:
	@zip -r $(TARGET).zip .git/ .gitignore $(SOURCES) README.md Makefile

.PHONY: all clean pack
