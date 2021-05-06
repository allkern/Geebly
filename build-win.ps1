$SDL2_INCLUDE_DIR = $args[0]
$SDL2_LIB_DIR = $args[1]
$LGW_INCLUDE_DIR = $args[2]
$IMGUI_SDL_DIR = $args[3]
$IMGUI_DIR = $args[4]
$IMPLOT_DIR = $args[5]
$VERSION_TAG = git describe --always --tags --abbrev=0
$COMMIT_HASH = git rev-parse --short HEAD

md -Force -Path build > $null

c++ -c src/geebly.cpp -o build/geebly.o -I"`"$($SDL2_INCLUDE_DIR)`"" -I"`"$($LGW_INCLUDE_DIR)`"" -I"`"$($IMGUI_DIR)`"" -I"`"$($IMGUI_SDL_DIR)`"" -I"`"$($IMPLOT_DIR)`"" -std=c++2a -m64 -mbmi2 -Ofast -Wno-format -Wno-narrowing -D GEEBLY_VERSION_TAG=$VERSION_TAG -D GEEBLY_COMMIT_HASH=$COMMIT_HASH -Wunused

c++ precomp/implot.o precomp/implot_items.o precomp/implot_demo.o precomp/imgui.o precomp/imgui_demo.o precomp/imgui_draw.o precomp/imgui_tables.o precomp/imgui_widgets.o precomp/imgui_sdl.o build/geebly.o -o build/geebly.exe -L"`"$($SDL2_LIB_DIR)`"" -limm32 -m64 -lSDL2main -lSDL2

del "build\geebly.o"