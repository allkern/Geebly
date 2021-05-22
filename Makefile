VERSION_TAG := $(shell git describe --always --tags --abbrev=0)
COMMIT_HASH := $(shell git rev-parse --short HEAD)

EXE    := geebly
SRC    := frontend
BIN    := bin
BUILD  := build
IMGUI_DIR := imgui
SOURCES := $(IMGUI_DIR)/imgui.cpp
SOURCES += $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_DIR)/examples/libs/gl3w/GL/gl3w.c

.PHONY: $(EXE)
geebly: $(BIN)/$(EXE)

$(BIN)/$(EXE): $(BUILD)/$(EXE).o
	c++ *.o $(BUILD)/$(EXE).o -o $(BIN)/$(EXE) -ldl -lGL -Ofast -m64 -lSDL2 -g -pthread

	rm -f build/$(EXE).o

$(BUILD)/$(EXE).o: $(SRC)/$(EXE).cpp
	mkdir -p $(BUILD)
	mkdir -p $(BIN)

	c++ -c \
		-I$(IMGUI_DIR)/examples/libs/gl3w \
		-I$(IMGUI_DIR) \
		-I/usr/include/SDL2 \
		-ldl \
		$(SOURCES)

	c++ -c $(SRC)/$(EXE).cpp -o $(BUILD)/$(EXE).o -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I$(IMGUI_DIR)/examples/libs/gl3w \
		-DIMGUI_IMPL_OPENGL_LOADER_GL3W \
		-DGEEBLY_VERSION_TAG=$(VERSION_TAG) \
		-DGEEBLY_COMMIT_HASH=$(COMMIT_HASH) \
		-Ofast -m64 -mbmi2 -Wno-format -Wno-narrowing -g -ldl

clean:
	rm -rf $(BUILD)
	rm -rf $(BIN)

install:
	sudo cp -rf $(BIN)/$(EXE) /usr/bin/$(EXE)

install-dev:
	sudo cp -rf geebly /usr/include/geebly/

clean-dev:
	sudo rm -rf /usr/include/geebly/
