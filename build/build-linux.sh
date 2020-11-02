cd ../src
c++ -c geebly.cpp -o ../build/geebly.o -std=c++2a -Ofast -m64 -Wno-format

# Link everything into a neat little file, also link OpenGL and X11
cd ../build
c++ geebly.o imgui.o imgui_draw.o imgui_widgets.o imgui-SFML.o -o geebly -Ofast -m64 -std=c++2a -lsfml-graphics -lsfml-window -lsfml-system -lGL -lX11

# Cleanup
cd ..
mv "build/geebly" "geebly"
rm "build/geebly.o"

echo "Geebly:"
time ./geebly