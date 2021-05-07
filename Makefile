VERSION_TAG := $(shell git describe --always --tags --abbrev=0)
COMMIT_HASH := $(shell git rev-parse --short HEAD)

SRC   := src
BIN   := bin
BUILD := build

.PHONY: geebly
geebly: $(BIN)/geebly

$(BIN)/geebly: $(BUILD)/geebly.o
	c++ $(BUILD)/geebly.o -o $(BIN)/geebly -Ofast -m64 -lSDL2

$(BUILD)/geebly.o: $(SRC)/geebly.cpp
	mkdir -p $(BUILD)
	mkdir -p $(BIN)

	c++ -c $(SRC)/geebly.cpp -o $(BUILD)/geebly.o -Ofast -m64 -mbmi2 -Wno-format -Wno-narrowing \
		-D GEEBLY_NO_DEBUGGER \
		-D GEEBLY_VERSION_TAG=$(VERSION_TAG) \
		-D GEEBLY_COMMIT_HASH=$(COMMIT_HASH)

clean:
	rm -rf $(BUILD)
	rm -rf $(BIN)

install:
	sudo cp -rf $(BIN)/geebly /usr/bin/geebly
