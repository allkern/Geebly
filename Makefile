VERSION_TAG := $(shell git describe --always --tags --abbrev=0)
COMMIT_HASH := $(shell git rev-parse --short HEAD)

build/geebly: build/geebly.o
	c++ build/geebly.o -o build/geebly -Ofast -m64 -lSDL2main -lSDL2
	rm -f build/geebly.o

build/geebly.o: src/geebly.cpp
	mkdir build

	c++ -c src/geebly.cpp -o build/geebly.o -Ofast -m64 -mbmi2 -Wno-format -Wno-narrowing -D GEEBLY_NO_DEBUGGER -D GEEBLY_VERSION_TAG=$(VERSION_TAG) -D GEEBLY_COMMIT_HASH=$(COMMIT_HASH)

clean:
	rm -rf build

install:
	sudo cp -rf build/geebly /usr/bin/geebly
