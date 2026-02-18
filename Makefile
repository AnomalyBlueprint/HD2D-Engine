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

# ============================================================
# Content Pipeline Tools
# ============================================================
TOOLS_DIR := tools/tool-server

.PHONY: tools-deps edit-bestiary edit-ui

tools-deps:
	@if [ ! -d "$(TOOLS_DIR)/node_modules" ]; then \
		echo "Installing tool dependencies..."; \
		cd $(TOOLS_DIR) && npm install; \
	fi

edit-bestiary: tools-deps
	@echo "Starting Bestiary Architect..."
	@cd $(TOOLS_DIR) && node open-browser.js http://localhost:3001/bestiary/ &
	@cd $(TOOLS_DIR) && node server.js

edit-ui: tools-deps
	@echo "Starting UI Architect..."
	@cd $(TOOLS_DIR) && node open-browser.js http://localhost:3001/ui/ &
	@cd $(TOOLS_DIR) && node server.js