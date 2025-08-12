import os
import subprocess

# Settings
Compiler = "g++"
Optimization = "-O3"
Debug = "" # -g
Last_Compiled = "Last_Compiled.txt"
Source_Includes = ['-I', 'MyCraft/src', '-I', 'MyCraft/src/Render', '-I', 'MyCraft/src/World', '-I', 'MyCraft/src/Player', '-I', 'MyCraft/src/Utils', '-I', 'MyCraft/src/Shader_Utils',
                    '-IMyCraft/Include', '-IMyCraft/Include/ImGui', '-IMyCraft/Include/ImGui/backends']
Flags = ['-std=c++17', '-LMyCraft/lib', '-lglfw3', '-lgdi32', '-lopengl32', '-static-libstdc++', '-static-libgcc', '-static']
Error = False
Destination = "../Mycraft.exe"

# Paths
Base = "MyCraft/"
PreCompiled = "src/PreCompiled"
Source_Folders = ["src/Player", "src/Render", "src/Shader_Utils", "src/Utils", "src/World"]
Source_Files = ["src/Main.cpp", "Include/ImGui/imgui_demo.cpp", "Include/ImGui/imgui_draw.cpp", "Include/ImGui/backends/imgui_impl_glfw.cpp",
                 "Include/ImGui/backends/imgui_impl_opengl3.cpp", "Include/ImGui/imgui_tables.cpp", "Include/ImGui/imgui_widgets.cpp", "Include/ImGui/imgui.cpp"]

def Print_Red(text):
    print(f"\033[91m{text}\033[0m")

def Get_Files(File_List):
    global Error
    if File_List:
        To_Compile_Files = []
        for Script in File_List:
            Path = Base + Script
            if os.path.exists(Path):
                To_Compile_Files.append(Path)
            else:
                print(f"Error: {Path} Does not Exist!")
                Error = True
    return To_Compile_Files

def Get_Files_From_Folder(Folder_List):
    if Folder_List:
        Folder_List = Base + Folder_List
        List_Of_Files = []
        List_Of_Something = os.listdir(Folder_List)
        for Item in List_Of_Something:
            if Item.endswith((".cpp", ".c", ".hpp", ".h")):
                List_Of_Files.append(os.path.join(Folder_List, Item))
        return List_Of_Files

def Get_All_Files(Clean):
    To_Compile_fr = []
    All_Files = Get_Files(Source_Files)
    if Source_Folders:
        for Folder in Source_Folders:
            All_Files.extend(Get_Files_From_Folder(Folder))
    for File in All_Files:
        if os.path.exists(File):
            To_Compile_fr.extend(Set_Last_Modified(File, Clean))
    return To_Compile_fr

def Set_Last_Modified(File, Clean):
    File = File.replace("\\", "/")  # normalizacja ścieżki
    To_Compile = []
    IsFileChanged = os.path.getmtime(File)
    updated = False

    with open(Last_Compiled, "a+", encoding="utf-8") as f:
        f.seek(0)
        Lines = f.readlines()

        for i, Line in enumerate(Lines):
            if File in Line:
                Time = Line.split("=")
                if (len(Time) > 1 and float(Time[1].strip()) != IsFileChanged) or Clean:
                    Lines[i] = f"{File}={IsFileChanged}\n"
                    if not File.replace(".hpp", ".cpp") in To_Compile:
                        To_Compile.append(File.replace(".hpp", ".cpp"))
                updated = True
                break

        if not updated:
            Lines.append(f"{File}={IsFileChanged}\n")
            if not File.replace(".hpp", ".cpp") in To_Compile:
                To_Compile.append(File.replace(".hpp", ".cpp"))

        f.seek(0)
        f.truncate()
        f.writelines(Lines)

    return To_Compile

def Compile(Files):
    for File in Files:
        obj_file = os.path.join(Base, PreCompiled, os.path.splitext(os.path.basename(File))[0] + ".o")
        cmd_Command = [Compiler] + Source_Includes + ["-c", File, "-o", obj_file] + [Optimization] + Flags
        print("Compiling:", " ".join(cmd_Command))
        result = subprocess.run(cmd_Command, capture_output=True, text=True)
        if result.returncode == 0:
            print(f"Object {File} was compiled to {obj_file} with {Optimization} flag")
        else:
            print("Compilation Error:")
            print(result.stderr)

def Link():
    obj_dir = os.path.join(Base, PreCompiled)
    objects = [os.path.join(obj_dir, f) for f in os.listdir(obj_dir) if f.endswith(".o")]
    if not objects:
        print("Nothing to Link")
        return
    
    output_exe = os.path.join(Base, Destination)  # na Windows, na Linuxie byłoby "MyCraft"
    cmd = [Compiler] + objects + [Optimization] + Flags + ["-o", output_exe]
    
    print("Linking:", " ".join(cmd))
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode == 0:
        print(f"Sukces! Utworzono {output_exe}")
        subprocess.run([output_exe])
    else:
        Print_Red("Linking Error:")
        Print_Red(result.stderr)

Clean = False#(True if (input("Do you want to clean?") == "1") else False)
Files = Get_All_Files(Clean)
Compile(Files)
Link()