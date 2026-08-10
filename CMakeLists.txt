cmake_minimum_required(VERSION 3.31)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

project(MyCraft C CXX)

add_subdirectory(MyCraft/Include/glad)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/MyCraft/src/*.cpp")

set(WINDOWS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/MyCraft/src/Utils/resource.rc")

if(WIN32)
    add_executable(${CMAKE_PROJECT_NAME} ${MY_SOURCES} ${WINDOWS_ICON})
else()
    add_executable(${CMAKE_PROJECT_NAME} ${MY_SOURCES})
endif()

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/MyCraft/Include"
        "${CMAKE_CURRENT_SOURCE_DIR}/MyCraft/src"
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        -march=native
        -g
        -O3
        #-Werror
        #-DNDEBUG
        #-Wall
)

target_link_options(MyCraft PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glad
        ${CMAKE_SOURCE_DIR}/MyCraft/Lib/libglfw3.a
        #${CMAKE_SOURCE_DIR}/MyCraft/Lib/lib-vc2022/glfw3.lib
        opengl32
        gdi32
        winmm
        ws2_32
        mswsock
        wsock32
)