@echo off

cd ../src
c++ -c geebly.cpp -o ../build/geebly.o -I"C:\\Users\\Lisandro\\Desktop\\Development\\Libraries\\SDL2\\include" -I"C:\\Users\\Lisandro\\Desktop\\Development\\Libraries\\lgw" -std=c++2a -Ofast -Wno-format -Wno-narrowing

cd ../build
c++ geebly.o -o geebly.exe -L"C:\\Users\\Lisandro\\Desktop\\Development\\Libraries\\SDL2\\lib\\x64" -lSDL2main -lSDL2

del geebly.o