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
set ImGui=Include/ImGui
set ImGuiBack=Include/ImGui/backends

:: ------Sources-----
:: Utils
set SUtils=%Utils%/FPS.cpp %Utils%/glad.c %Utils%/resource.o %Utils%/Settings.cpp %Utils%/Function.cpp

:: World
set SWorld=%World%/Chunk.cpp %World%/Terrain.cpp %World%/Generation.cpp

:: Render
set SRender=%Render%/Camera.cpp %Render%/Mesh.cpp

:: Player
set SPlayer=%Player%/Colisions.cpp %Player%/Movement.cpp

:: Shader Utils
set SShader=%Shader_Utils%/Shader.cpp

:: ImGui
set SImGui=%ImGui%/imgui.cpp %ImGui%/imgui_demo.cpp %ImGui%/imgui_draw.cpp %ImGui%/imgui_tables.cpp %ImGui%/imgui_widgets.cpp %ImGuiBack%/imgui_impl_glfw.cpp %ImGuiBack%/imgui_impl_opengl3.cpp

:: -----Compilation--
g++ %Flags1% src/main.cpp %SUtils% %SWorld% %SRender% %SPlayer% %SShader% %SImGui% %Flags2% -o ../MyCraft.exe

cd ..
Mycraft.exe
pause
