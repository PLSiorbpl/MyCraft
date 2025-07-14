cd Mycraft
g++ src/main.cpp src/glad.c src/resource.o -Iinclude -Llib -lglfw3 -lgdi32 -lopengl32 -static-libstdc++ -static-libgcc -o ../MyCraft.exe
cd ..
Mycraft.exe
pause
