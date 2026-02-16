# HD2D Engine - Universal Makefile (macOS Adapted)
# Automatically finds sources in src/Engine and src/Game

# Compiler Settings
CXX      := clang++
# -MMD -MP: Generates dependency files (.d)
# Includes for Homebrew (M1/M2/M3 Macs)
CXXFLAGS := -std=c++17 -Wall -Wextra -g -D_THREAD_SAFE -MMD -MP \
            -Iinclude -Iinclude/vendor \
            -I/opt/homebrew/include -I/opt/homebrew/include/SDL2

# Linker Flags for macOS
LDFLAGS  := -L/opt/homebrew/lib -lSDL2 -lGLEW -framework OpenGL

# Directories
SRC_DIR   := src
BUILD_DIR := build
BIN_DIR   := bin

# Automatically find all .cpp files in src/ and its subdirectories
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')

# Generate corresponding .o file paths in build/ dir
# Example: src/main.cpp -> build/src/main.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# Generate dependency files (.d) to track header changes
DEPS := $(OBJS:.o=.d)

# The Output Binary
TARGET := $(BIN_DIR)/engine_3d

# --- Rules ---

# Main Build Target
$(TARGET): $(OBJS)
	@echo "Linking..."
	@mkdir -p $(dir $@)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build Complete: $(TARGET)"

# Compile C++ Source
# % matches "src/Path/To/File"
$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean Build
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Run the Game
run: $(TARGET)
	./$(TARGET)

# Include dependencies (so editing a .h file triggers a rebuild)
-include $(DEPS)

.PHONY: clean run