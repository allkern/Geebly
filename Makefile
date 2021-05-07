VERSION_TAG := $(shell git describe --always --tags --abbrev=0)
COMMIT_HASH := $(shell git rev-parse --short HEAD)

SRC   := src
BUILD := build

$(BUILD)/geebly: $(BUILD)/geebly.o
	c++ $(BUILD)/geebly.o -o $(BUILD)/geebly -Ofast -m64 -lSDL2
	rm -f $(BUILD)/geebly.o

$(BUILD)/geebly.o: $(SRC)/geebly.cpp
	mkdir -p $(BUILD)

	c++ -c $(SRC)/geebly.cpp -o $(BUILD)/geebly.o -Ofast -m64 -mbmi2 -Wno-format -Wno-narrowing \
		-D GEEBLY_NO_DEBUGGER \
		-D GEEBLY_VERSION_TAG=$(VERSION_TAG) \
		-D GEEBLY_COMMIT_HASH=$(COMMIT_HASH)

clean:
	rm -rf $(BUILD)

install:
	sudo cp -rf $(BUILD)/geebly /usr/bin/geebly
