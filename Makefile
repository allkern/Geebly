build/geebly: build/geebly.o
	c++ build/geebly.o -o build/geebly -Ofast -m64 -lSDL2main -lSDL2
	rm -f build/geebly.o

build/geebly.o: src/geebly.cpp
	mkdir build
	c++ -c src/geebly.cpp -o build/geebly.o -Ofast -m64 -Wno-format -Wno-narrowing -D GEEBLY_VERSION_TAG=$(git describe --tags --abbrev=0) -D GEEBLY_COMMIT_ID=$(git rev-parse --short HEAD)

clean:
	rm -rf build

install:
	sudo cp -rf build/geebly /usr/bin/geebly
