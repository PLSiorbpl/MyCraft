cd Mycraft
g++ src/main.cpp src/Utils/FPS.cpp src/Utils/glad.c src/Utils/resource.o ^
    src/Render/Camera.cpp src/Utils/Settings.cpp src/Shader_Utils/Shader.cpp ^
    src/Player/Colisions.cpp src/Player/Movement.cpp src/World/Chunk.cpp ^
    src/Utils/Function.cpp ^
    -I src -I src/Render -I src/World -I src/Player -I src/Utils -I src/Shader_Utils ^
    -Iinclude -Llib -lglfw3 -lgdi32 -lopengl32 ^
    -static-libstdc++ -static-libgcc -static -o ../MyCraft.exe
cd ..
Mycraft.exe
pause
