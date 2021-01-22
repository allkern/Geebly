@echo off

cd ../src
g++ -c geebly.cpp -o ../build/geebly.o -I"sfml\include" -std=c++2a -Ofast -Wno-format -Wno-narrowing

cd ../build
g++ geebly.o imgui_win.o imgui_draw_win.o imgui_widgets_win.o imgui-SFML_win.o -o geebly.exe -Ofast -std=c++2a -L"C:\Users\Lisandro\Desktop\Development\Libraries\SFML-2.5.1-32\lib" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lopengl32

del geebly.o