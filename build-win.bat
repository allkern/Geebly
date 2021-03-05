@echo off

set SDL2_INCLUDE_DIR="%1"
set SDL2_LIB_DIR="%2"
set LGW_INCLUDE_DIR="%3"

mkdir build
c++ -c src/geebly.cpp -o build/geebly.o -I%SDL2_INCLUDE_DIR% -I%LGW_INCLUDE_DIR% -std=c++2a -m64 -Ofast -Wno-format -Wno-narrowing -D GEEBLY_VERSION_MAJOR=0 -D GEEBLY_VERSION_MINOR=7 -D GEEBLY_VERSION_CLASS=b -D GEEBLY_COMMIT_ID=50e4056

c++ build/geebly.o -o build/geebly.exe -L%SDL2_LIB_DIR% -m64 -lmingw32 -lSDL2main -lSDL2

del "build\geebly.o"
