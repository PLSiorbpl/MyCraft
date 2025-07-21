cd Mycraft
g++ src/main.cpp src/Utils/FPS.cpp src/Utils/glad.c src/Utils/resource.o ^
    -Iinclude -Llib -lglfw3 -lgdi32 -lopengl32 ^
    -static-libstdc++ -static-libgcc -static -o ../MyCraft.exe
cd ..
Mycraft.exe
pause
