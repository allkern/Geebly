$GEEBLY_INCLUDE_DIR = "."
$LGW_INCLUDE_DIR    = "C:\Users\Lisandro\Desktop\Development\Libraries\lgw"
$IMPLOT_DIR         = "C:\Users\Lisandro\Desktop\Development\Libraries\implot"
$IMGUI_DIR          = "C:\Users\Lisandro\Desktop\Development\Libraries\imgui-1.83"
$SDL2_DIR           = "C:\Users\Lisandro\Desktop\Development\Libraries\SDL2"

$VERSION_TAG = git describe --always --tags --abbrev=0
$COMMIT_HASH = git rev-parse --short HEAD

md -Force -Path build > $null

Write-Output "Building GUI executable..."

c++ -I"`"$($IMGUI_DIR)`"" `
    -I"`"$($IMGUI_DIR)\backends`"" `
    -I"`"$($IMGUI_DIR)\examples\libs\gl3w`"" `
    -I"`"$($GEEBLY_INCLUDE_DIR)`"" `
    -I"`"$($LGW_INCLUDE_DIR)`"" `
    -I"`"$($SDL2_DIR)\include\SDL2`"" `
    -I"`"$($SDL2_DIR)\include`"" `
    -I"`"$($IMPLOT_DIR)`"" `
    "*.o" `
    "res/*.res" `
    frontend\geebly.cpp `
    -DGEEBLY_VERSION_TAG="`"$($VERSION_TAG)`"" `
    -DGEEBLY_COMMIT_HASH="`"$($COMMIT_HASH)`"" `
    -o bin\geebly_gui_32.exe `
    -L"`"$($SDL2_DIR)\lib\x64`"" `
    -DIMGUI_IMPL_OPENGL_LOADER_GL3W `
    -limm32 -mbmi2 -lSDL2main -lSDL2 -lopengl32 -lSDL2_ttf -fpermissive -lcomdlg32 -g -std=c++2a -mwindows

Write-Output "Building console executable..."

c++ -I"`"$($IMGUI_DIR)`"" `
    -I"`"$($IMGUI_DIR)\backends`"" `
    -I"`"$($IMGUI_DIR)\examples\libs\gl3w`"" `
    -I"`"$($GEEBLY_INCLUDE_DIR)`"" `
    -I"`"$($LGW_INCLUDE_DIR)`"" `
    -I"`"$($SDL2_DIR)\include\SDL2`"" `
    -I"`"$($SDL2_DIR)\include`"" `
    -I"`"$($IMPLOT_DIR)`"" `
    "*.o" `
    "res/*.res" `
    frontend\geebly.cpp `
    -DGEEBLY_VERSION_TAG="`"$($VERSION_TAG)`"" `
    -DGEEBLY_COMMIT_HASH="`"$($COMMIT_HASH)`"" `
    -o bin\geebly_32.exe `
    -L"`"$($SDL2_DIR)\lib\x64`"" `
    -DIMGUI_IMPL_OPENGL_LOADER_GL3W `
    -limm32 -mbmi2 -lSDL2main -lSDL2 -lopengl32 -lSDL2_ttf -fpermissive -lcomdlg32 -g -std=c++2a