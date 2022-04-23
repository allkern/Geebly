$GEEBLY_INCLUDE_DIR = "."
$LGW_INCLUDE_DIR    = "lgw"
$IMGUI_DIR          = "imgui"
$SDL2_DIR           = "sdl2/SDL2-2.0.20/"

$VERSION_TAG = git describe --always --tags --abbrev=0
$COMMIT_HASH = git rev-parse --short HEAD

md -Force -Path bin > $null

c++ -c -I"`"$($IMGUI_DIR)`"" `
       -I"`"$($IMGUI_DIR)\backends`"" `
       -I"`"$($IMGUI_DIR)\examples\libs\gl3w`"" `
       -I"`"$($GEEBLY_INCLUDE_DIR)`"" `
       -I"`"$($LGW_INCLUDE_DIR)`"" `
       -I"`"$($SDL2_DIR)\include\SDL2`"" `
       -I"`"$($SDL2_DIR)\include`"" `
       "`"$($IMGUI_DIR)\backends\imgui_impl_sdl.cpp`"" `
       "`"$($IMGUI_DIR)\backends\imgui_impl_opengl3.cpp`"" `
       "`"$($IMGUI_DIR)\*.cpp`"" `
       "`"$($IMGUI_DIR)\examples\libs\gl3w\GL\gl3w.c`"" `
       -L"`"$($SDL2_DIR)\lib\x64`"" `
       -DIMGUI_IMPL_OPENGL_LOADER_GL3W `
       -limm32 -m64 -mbmi2 -lSDL2main -lSDL2 -lopengl32 -fpermissive -lcomdlg32

c++ -I"`"$($IMGUI_DIR)`"" `
    -I"`"$($IMGUI_DIR)\backends`"" `
    -I"`"$($IMGUI_DIR)\examples\libs\gl3w`"" `
    -I"`"$($GEEBLY_INCLUDE_DIR)`"" `
    -I"`"$($LGW_INCLUDE_DIR)`"" `
    -I"`"$($SDL2_DIR)\include\SDL2`"" `
    -I"`"$($SDL2_DIR)\include`"" `
    "*.o" `
    frontend\geebly.cpp `
    -DGEEBLY_VERSION_TAG="`"$($VERSION_TAG)`"" `
    -DGEEBLY_COMMIT_HASH="`"$($COMMIT_HASH)`"" `
    -o bin\geebly.exe `
    -L"`"$($SDL2_DIR)\lib\x64`"" `
    -DIMGUI_IMPL_OPENGL_LOADER_GL3W `
    -limm32 -m64 -mbmi2 -lSDL2main -lSDL2 -lSDL2_ttf -lopengl32 -fpermissive -lcomdlg32
