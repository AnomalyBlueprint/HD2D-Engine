# --- AUTO-DETECT CORES FOR SPEED ---
NPROCS = $(shell sysctl -n hw.ncpu 2>/dev/null || nproc)
JOBS = $(shell echo $$(($(NPROCS) - 1)))
MAKEFLAGS += -j$(JOBS)

# --- COMPILER SETTINGS ---
CXX = clang++
# -MMD -MP: Generates dependency files (.d) so you don't recompile everything unnecessarily
# Includes: /opt/homebrew is for M2 Macs
CXXFLAGS = -std=c++17 -Wall -D_THREAD_SAFE -MMD -MP \
           -Iinclude \
           -I/opt/homebrew/include \
           -I/opt/homebrew/include/SDL2

# Linker: Links SDL2, GLEW, and the OpenGL Framework
LDFLAGS = -L/opt/homebrew/lib -lSDL2 -lGLEW -framework OpenGL

# --- DIRECTORIES ---
SRC_DIR = src
BUILD_DIR = build
TARGET = engine_3d

# Recursive search for .cpp files (in src/ and subfolders like src/Services/)
SRCS = $(shell find $(SRC_DIR) -name '*.cpp')
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))
DEPS = $(OBJS:.o=.d)

# --- RULES ---

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile C++ to Object (Preserving folder structure in build/)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: clean