cd Mycraft
setlocal

:: ------Flags-------
set Flags1=-I src -I src/Render -I src/World -I src/Player -I src/Utils -I src/Shader_Utils -IInclude -IInclude/ImGui -IInclude/ImGui/backends 
set Flags2=-O3 -std=c++17 -Llib -lglfw3 -lgdi32 -lopengl32 -static-libstdc++ -static-libgcc -static


:: ------Path--------
set Utils=src/Utils
set Render=src/Render
set Player=src/Player
set World=src/World
set Shader_Utils=src/Shader_Utils
set Obj=Include/obj

:: ------Sources-----
:: Utils
set SUtils=%Utils%/FPS.cpp %Utils%/resource.o %Utils%/Settings.cpp %Utils%/Function.cpp

:: World
set SWorld=%World%/Chunk.cpp %World%/Terrain.cpp %World%/Generation.cpp

:: Render
set SRender=%Render%/Camera.cpp %Render%/Mesh.cpp

:: Player
set SPlayer=%Player%/Colisions.cpp %Player%/Movement.cpp

:: Shader Utils
set SShader=%Shader_Utils%/Shader.cpp

:: -----Compilation--
g++ %Flags1% src/main.cpp %SUtils% %SWorld% %SRender% %SPlayer% %SShader% %Obj%/*.o %Flags2% -o ../MyCraft.exe

cd ..
Mycraft.exe
pause
