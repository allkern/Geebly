git clone "https://github.com/ocornut/imgui"
git clone "https://github.com/skaslev/gl3w"
git clone "https://github.com/Lycoder/lgw"

python3 "gl3w/gl3w_gen.py"

md -Force -Path "imgui/examples/libs/gl3w/GL" > $null

cp -Force "src/gl3w.c","include/GL/gl3w.h","include/GL/glcorearb.h" "imgui/examples/libs/gl3w/GL/"

Invoke-WebRequest "https://www.libsdl.org/release/SDL2-devel-2.0.20-VC.zip" -OutFile "./sdl2.zip"
Invoke-WebRequest "https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.0.18/SDL2_ttf-devel-2.0.18-VC.zip" -OutFile "./sdl2-ttf.zip"

Expand-Archive -Force "./sdl2.zip" -DestinationPath "sdl2/"
Expand-Archive -Force "./sdl2-ttf.zip" -DestinationPath "sdl2-ttf/"

del "./sdl2.zip","./sdl2-ttf.zip"

cp -Force "sdl2-ttf/SDL2_ttf-2.0.18/include/SDL_ttf.h" "sdl2/SDL2-2.0.20/include/"
cp -Force "sdl2-ttf/SDL2_ttf-2.0.18/lib/x64/SDL2_ttf.*" "sdl2/SDL2-2.0.20/lib/x64/"

git clone "https://github.com/Lycoder/sdl_shader"

cp -Force "sdl_shader/SDL_shader.h" "sdl2/SDL2-2.0.20/include/"

cp -Force "sdl2/SDL2-2.0.20/lib/x64/*.dll" "."