$GEEBLY_INCLUDE_DIR = "."
$LGW_INCLUDE_DIR    = "C:\Users\Lisandro\Desktop\Development\Libraries\lgw"
$IMPLOT_DIR         = "C:\Users\Lisandro\Desktop\Development\Libraries\implot"
$IMGUI_DIR          = "C:\Users\Lisandro\Desktop\Development\Libraries\imgui-1.83"
$SDL2_DIR           = "C:\Users\Lisandro\Desktop\Development\Libraries\SDL2"

c++ -c -I"`"$($IMGUI_DIR)`"" `
       -I"`"$($IMGUI_DIR)\backends`"" `
       -I"`"$($IMGUI_DIR)\examples\libs\gl3w`"" `
       -I"`"$($GEEBLY_INCLUDE_DIR)`"" `
       -I"`"$($LGW_INCLUDE_DIR)`"" `
       -I"`"$($SDL2_DIR)\include\SDL2`"" `
       -I"`"$($SDL2_DIR)\include`"" `
       -I"`"$($IMPLOT_DIR)`"" `
       "`"$($IMGUI_DIR)\backends\imgui_impl_sdl.cpp`"" `
       "`"$($IMGUI_DIR)\backends\imgui_impl_opengl3.cpp`"" `
       "`"$($IMGUI_DIR)\*.cpp`"" `
       "`"$($IMPLOT_DIR)\*.cpp`"" `
       "`"$($IMGUI_DIR)\examples\libs\gl3w\GL\gl3w.c`"" `
       -L"`"$($SDL2_DIR)\lib\x64`"" `
       -DIMGUI_IMPL_OPENGL_LOADER_GL3W `
       -limm32 -m64 -mbmi2 -lSDL2main -lSDL2 -lopengl32 -fpermissive -lcomdlg32