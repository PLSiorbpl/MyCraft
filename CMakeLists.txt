cmake_minimum_required(VERSION 3.31)
project(MyCraft)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(MyCraft/Include/glad)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/MyCraft/src/*.cpp")
add_executable(${CMAKE_PROJECT_NAME} ${MY_SOURCES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/MyCraft/Include"
        "${CMAKE_CURRENT_SOURCE_DIR}/MyCraft/src/GUI"
        "${CMAKE_CURRENT_SOURCE_DIR}/MyCraft/src/Network"
        "${CMAKE_CURRENT_SOURCE_DIR}/MyCraft/src/Player"
        "${CMAKE_CURRENT_SOURCE_DIR}/MyCraft/src/Render"
        "${CMAKE_CURRENT_SOURCE_DIR}/MyCraft/src/Shader_Utils"
        "${CMAKE_CURRENT_SOURCE_DIR}/MyCraft/src/World"
        "${CMAKE_CURRENT_SOURCE_DIR}/MyCraft/src/Utils"
        "${CMAKE_CURRENT_SOURCE_DIR}/MyCraft/src"
)

if(PRODUCTION_BUILD)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC PRODUCTION_BUILD=1)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -O0 -march=native)
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
else()
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC PRODUCTION_BUILD=0)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -g )
endif()

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/MyCraft/Assets
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/MyCraft/Assets

        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/MyCraft/shaders
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/MyCraft/shaders
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glad
        ${CMAKE_SOURCE_DIR}/MyCraft/Lib/libglfw3.a
        opengl32
        gdi32
        winmm
        ws2_32
        mswsock
        wsock32
)