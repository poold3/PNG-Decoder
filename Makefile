# FOLLOW THE STEPS BELOW TO USE THIS MAKEFILE

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -Wno-error=unused-parameter -Wno-error=unused-variable
LDFLAGS = -lz

# Directories
SRC_DIR = src
HEADER_DIR = headers
LIB_DIR = lib
BUILD_DIR = build

# Source files and object files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))

# 1. ADJUST PATHS TO EXTERNAL LIBRARIES
ZLIB_DIR = path/to/zlib

# Include directories for external libraries
ZLIB_INCLUDE = -I$(ZLIB_DIR)

# Target: YourLibrary.so (shared library)
LIB_NAME = PNG-Decoder
LIB_EXT = so  # Default extension for Unix-like systems
ifeq ($(OS), Windows_NT)
    LIB_EXT = dll  # Change to .dll for Windows
endif
LIB_SO = $(LIB_DIR)/lib$(LIB_NAME).$(LIB_EXT)

all: $(LIB_SO)

$(LIB_SO): $(OBJ_FILES)
	@mkdir -p $(LIB_DIR)
	$(CXX) -shared -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(ASIO_INCLUDE) $(JSON_INCLUDE) $(WEBSOCKETPP_INCLUDE) -I$(HEADER_DIR) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR)

.PHONY: all clean