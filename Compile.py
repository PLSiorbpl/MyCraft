import os
import sys
import subprocess
import time
from concurrent.futures import ProcessPoolExecutor, as_completed

# Settings
Compiler = "g++"
Optimization = "-O0"
Debug = "-g" # -g
Open_Game = True
Last_Compiled = "Last_Compiled.txt"
Source_Includes = ['-I', 'MyCraft/src', '-I', 'MyCraft/src/Render', '-I', 'MyCraft/src/World', '-I', 'MyCraft/src/Player', '-I', 'MyCraft/src/Utils', '-I', 'MyCraft/src/Shader_Utils',
                    '-IMyCraft/Include']#, '-IMyCraft/Include/ImGui', '-IMyCraft/Include/ImGui/backends']
if sys.platform == "win32": # Windows
    Flags = ['-g', '-D_GLIBCXX_DEBUG', '-fno-omit-frame-pointer', '-std=c++17', '-LMyCraft/lib', '-lglfw3', '-lgdi32', '-lopengl32', '-static-libstdc++', '-static-libgcc', '-static']
    Destination = ["../Mycraft.exe"]
if sys.platform == "linux": # Linux
    Flags = ['-std=c++17', '-LMyCraft/lib', '-lglfw', '-lGL', '-lpthread', '-ldl', '-lX11', '-static-libstdc++', '-static-libgcc']
    Destination = ["../Mycraft-Linux"]
Error = False

# Paths
Base = "MyCraft/"
PreCompiled = ["src/PreCompiled"]
Source_Folders = [["src/Player", "src/Render", "src/Shader_Utils", "src/Utils", "src/World", "src/GUI"]]
Source_Files = [["src/Main.cpp"]]#, "Include/ImGui/imgui_demo.cpp", "Include/ImGui/imgui_draw.cpp", "Include/ImGui/backends/imgui_impl_glfw.cpp",
                 #"Include/ImGui/backends/imgui_impl_opengl3.cpp", "Include/ImGui/imgui_tables.cpp", "Include/ImGui/imgui_widgets.cpp", "Include/ImGui/imgui.cpp"]]

def Print_Red(text):
    print(f"\033[91m{text}\033[0m")

def Print_Green(text):
    print(f"\033[92m{text}\033[0m")

def Print_Yellow(text):
    print(f"\033[93m{text}\033[0m")

def Print_Blue(text):
    print(f"\033[94m{text}\033[0m")

def Print_Cyan(text):
    print(f"\033[96m{text}\033[0m")

# Getting Files
def Get_Files(File_List):
    To_Compile_Files = []
    if File_List:
        for Script in File_List:
            Path = Base + Script
            if os.path.exists(Path):
                To_Compile_Files.append(Path)
            else:
                Print_Red(f"Error: {Path} Does not Exist!")
    return To_Compile_Files

def Get_Files_From_Folder(Folder_List):
    List_Of_Files = []
    if Folder_List:
        Folder_List = Base + Folder_List
        List_Of_Something = os.listdir(Folder_List)
        for Item in List_Of_Something:
            if Item.endswith((".cpp", ".c")):
                List_Of_Files.append(os.path.join(Folder_List, Item))
        return List_Of_Files

# Prepare To Compile Files
def Get_All_Files(Clean, Src_Folder, Src_Files):
    To_Compile_fr = []
    All_Files = Get_Files(Src_Files)
    if Src_Folder:
        for Folder in Src_Folder:
            All_Files.extend(Get_Files_From_Folder(Folder))

    for File in All_Files:
        if os.path.exists(File):
            To_Compile_fr.extend(Set_Last_Modified(File, Clean))
    return To_Compile_fr

def Set_Last_Modified(File, Clean):
    File = File.replace("\\", "/")
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
                    if not File in To_Compile:
                        To_Compile.append(File)
                updated = True
                break

        if not updated:
            Lines.append(f"{File}={IsFileChanged}\n")
            if not File in To_Compile:
                To_Compile.append(File)

        f.seek(0)
        f.truncate()
        f.writelines(Lines)

    return To_Compile

# Compile Files
def Compile_File(File, PreCompile):
    obj_file = os.path.join(Base, PreCompile, os.path.splitext(os.path.basename(File))[0] + ".o")
    cmd_Command = [Compiler] + Source_Includes + ["-c", File, "-o", obj_file] + [Debug] + [Optimization] + Flags
    result = subprocess.run(cmd_Command, capture_output=True, text=True)
    if result.returncode == 0:
        Print_Green(f"Object {File} was compiled to {obj_file} with {Optimization} flag")
    else:
        Print_Red(f"Compilation Error in {File}:")
        Print_Red(result.stderr)
    return result.returncode

def Compile(Files, jobs, PreCompile):
    results = []
    with ProcessPoolExecutor(max_workers=jobs) as executor:
        future_to_file = {executor.submit(Compile_File, File, PreCompile): File for File in Files}
        for future in as_completed(future_to_file):
            file = future_to_file[future]
            try:
                result = future.result()
                results.append((file, result))
            except Exception as exc:
                Print_Red(f"{file} generated an exception: {exc}")
                results.append((file, 1))
    return results

def Link(Destination_exe, PreCompile):
    obj_dir = os.path.join(Base, PreCompile)
    if sys.platform == "win32": # Windows
        objects = [os.path.join(obj_dir, f) for f in os.listdir(obj_dir) if f.endswith(".o")]
    if sys.platform == "linux": # Linux
        objects = [os.path.join(obj_dir, f) for f in os.listdir(obj_dir) if f.endswith(".o") and f != "resource.o"]
    if not objects:
        Print_Red("Nothing to Link")
        return
    
    output_exe = os.path.join(Base, Destination_exe)
    cmd = [Compiler] + objects + [Optimization] + Flags + ["-o", output_exe]
    small_cmd = [Compiler] + objects + ["-o", output_exe]
    
    Print_Cyan(f"Linking: {small_cmd}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode == 0:
        Print_Green(f"Created: {output_exe}")
        return output_exe
    else:
        Print_Red("Linking Error:")
        Print_Red(result.stderr)

def Open(Path):
     if Open_Game and os.path.exists(Path):
        Print_Blue(f"Opening: {Path}")
        subprocess.run([Path])

def main():
    Print_Cyan(f"Max Cores: {os.cpu_count()}")
    Print_Cyan("1 - Yes | 0 - No")
    Clean = (True if (input("ReCompile All? :") == "1") else False)
    for i in range(len(Destination)):
        Print_Yellow(f"\n{Destination[i]}")

        t_files = time.perf_counter()
        Files = Get_All_Files(Clean, Source_Folders[i], Source_Files[i])
        Print_Cyan(f"Getting Files: {time.perf_counter() - t_files:.3f}s")

        t_compile = time.perf_counter()
        Compile(Files, os.cpu_count(), PreCompiled[i])
        Print_Cyan(f"Compiling: {time.perf_counter() - t_compile:.3f}s")

        t_link = time.perf_counter()
        Path = Link(Destination[i], PreCompiled[i])
        Print_Cyan(f"Linking: {time.perf_counter() - t_link:.3f}s")

        Open(Path)

if __name__ == "__main__":
    main()