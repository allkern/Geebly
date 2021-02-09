cd ../src
c++ -c geebly.cpp -o ../build/geebly.o -std=c++2a -Ofast -m64 -Wno-format -Wno-narrowing

# Link everything into a neat little file, also link OpenGL and X11
cd ../build
c++ geebly.o -o geebly -Ofast -m64 -std=c++2a -lSDL2main -lSDL2

# Cleanup
cd ..
mv "build/geebly" "geebly"
rm "build/geebly.o"