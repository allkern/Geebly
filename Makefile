VERSION_TAG := $(shell git describe --always --tags --abbrev=0)
COMMIT_HASH := $(shell git rev-parse --short HEAD)

EXE    := geebly
SRC    := frontend
BIN    := bin
BUILD  := build
IMGUI_DIR := imgui
LGW_DIR := lgw

SOURCES := $(IMGUI_DIR)/imgui.cpp
SOURCES += $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_DIR)/examples/libs/gl3w/GL/gl3w.c

.PHONY: $(EXE)
geebly: $(BIN)/$(EXE)

$(BIN)/$(EXE): $(BUILD)/$(EXE).o
	c++ *.o $(BUILD)/$(EXE).o -o $(BIN)/$(EXE) \
	-ldl -lGL -Ofast -m64 -lSDL2 -g -pthread \
	-m64 -mbmi2 -lSDL2main -lSDL2 -lSDL2_ttf -fpermissive

	rm -f build/$(EXE).o

$(BUILD)/$(EXE).o: $(SRC)/$(EXE).cpp
	mkdir -p $(BUILD)
	mkdir -p $(BIN)

	c++ -c \
		-I$(IMGUI_DIR)/examples/libs/gl3w \
		-I$(IMGUI_DIR) \
		-I$(LGW_DIR) \
		-I/usr/include/SDL2 \
		-ldl \
		-I"." \
		$(SOURCES)

	c++ -c $(SRC)/$(EXE).cpp -o $(BUILD)/$(EXE).o \
		-I"." \
		-I$(IMGUI_DIR) -I$(LGW_DIR) -I$(IMGUI_DIR)/backends -I$(IMGUI_DIR)/examples/libs/gl3w \
		-I/usr/include/SDL2 \
		-DIMGUI_IMPL_OPENGL_LOADER_GL3W \
		-DGEEBLY_VERSION_TAG=$(VERSION_TAG) \
		-DGEEBLY_COMMIT_HASH=$(COMMIT_HASH) \
		-Ofast -m64 -mbmi2 -Wno-format -Wno-narrowing -g -ldl -O3

environment:
# Clone imgui
	git clone https://github.com/ocornut/imgui

# Always update your apt!
	sudo apt update

# Install libsdl2 and libsdl2-ttf
	sudo apt install libsdl2-dev
	sudo apt install libsdl2-ttf-dev

# Get gl3w, generate sources and copy them to ImGui's dir
	git clone https://github.com/skaslev/gl3w
	python3 gl3w/gl3w_gen.py
	mkdir -p imgui/examples/libs/gl3w/GL
	cp src/gl3w.c include/GL/gl3w.h include/GL/glcorearb.h imgui/examples/libs/gl3w/GL

# Get lgw
	git clone https://github.com/Lycoder/lgw

# Get and install SDL_shader
	git clone https://github.com/Lycoder/sdl_shader
	cd sdl_shader; make install

# Now you're ready to make!

clean:
	rm -rf $(BUILD)
	rm -rf $(BIN)
	rm -rf "imgui"
	rm -rf "gl3w"
	rm -rf "src"
	rm -rf "include"
	rm -rf "sdl_shader"
	rm -rf *.o

install:
	sudo cp -rf $(BIN)/$(EXE) /usr/bin/$(EXE)

install-dev:
	sudo cp -rf geebly /usr/include/geebly/

clean-dev:
	sudo rm -rf /usr/include/geebly/
