@echo off

set SDL2_INCLUDE_DIR="%1"
set SDL2_LIB_DIR="%2"
set LGW_INCLUDE_DIR="%3"

for /f "tokens=*" %%a in ('git describe --tags --abbrev=0') do set VERSION_TAG=%%a
for /f "tokens=*" %%a in ('git rev-parse --short HEAD') do set COMMIT_HASH=%%a

echo %VERSION_TAG%
echo %COMMIT_HASH%

mkdir build
c++ -c src/geebly.cpp -o build/geebly.o -I%SDL2_INCLUDE_DIR% -I%LGW_INCLUDE_DIR% -std=c++2a -m64 -Ofast -Wno-format -Wno-narrowing -D GEEBLY_VERSION_TAG=%VERSION_TAG% -D GEEBLY_COMMIT_HASH=%COMMIT_HASH%

c++ build/geebly.o -o build/geebly.exe -L%SDL2_LIB_DIR% -m64 -lmingw32 -lSDL2main -lSDL2

del "build\geebly.o"
