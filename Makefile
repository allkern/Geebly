build/geebly: build/geebly.o
	c++ build/geebly.o -o build/geebly -Ofast -m64 -std=c++20 -lSDL2main -lSDL2
	rm -f build/geebly.o

build/geebly.o: src/geebly.cpp
	mkdir build
	c++ -c src/geebly.cpp -o build/geebly.o -std=c++20 -Ofast -m64 -Wno-format -Wno-narrowing

clean:
	rm -rf build

install:
	sudo cp -rf build/geebly /usr/bin/geebly
