$SDL2_INCLUDE_DIR = $args[0]
$SDL2_LIB_DIR = $args[1]
$LGW_INCLUDE_DIR = $args[2]
$VERSION_TAG = git describe --tags --abbrev=0
$COMMIT_HASH = git rev-parse --short HEAD

md -Force -Path build > $null

c++ -c src/geebly.cpp -o build/geebly.o -I"`"$($SDL2_INCLUDE_DIR)`"" -I"`"$($LGW_INCLUDE_DIR)`"" -std=c++2a -m64 -Ofast -Wno-format -Wno-narrowing -D GEEBLY_VERSION_TAG=$VERSION_TAG -D GEEBLY_COMMIT_HASH=$COMMIT_HASH

c++ build/geebly.o -o build/geebly.exe -L"`"$($SDL2_LIB_DIR)`"" -m64 -lmingw32 -lSDL2main -lSDL2

del "build\geebly.o"
