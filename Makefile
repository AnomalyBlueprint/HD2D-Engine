CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -Iinclude -Iinclude/vendor -I/usr/local/include -I/opt/local/include -I/opt/homebrew/include -I/opt/homebrew/Cellar/sqlite/3.51.2/include $(shell sdl2-config --cflags)
LDFLAGS  := -framework OpenGL -L/usr/local/lib -L/opt/local/lib -L/opt/homebrew/lib -lGLEW -lsqlite3 $(shell sdl2-config --libs)

# Recursive wildcard to find all .cpp files in src/Engine, src/Game, etc.
rwildcard=$(foreach d,$(wildcard $(1)*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
SRCS := $(call rwildcard,src/,*.cpp)
OBJS := $(SRCS:src/%.cpp=build/%.o)
DEPS := $(OBJS:.o=.d)

TARGET := bin/engine_3d

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	@echo "Linking..."
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build Complete: $(TARGET)"

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf build bin

run: $(TARGET)
	./$(TARGET)

-include $(DEPS)