cmake_minimum_required(VERSION 3.10)

project("OpenGL-Test-Program")


set(EXE_NAME "main")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OpenGL-Test-Program_SOURCE_DIR}/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OpenGL-Test-Program_SOURCE_DIR}/lib")

include_directories("${OpenGL-Test-Program_SOURCE_DIR}/include")
link_directories("${OpenGL-Test-Program_SOURCE_DIR}/lib")

add_subdirectory("lib")

add_executable(${EXE_NAME} "./src/mainloop.cpp" "./src/glad.c")

target_link_libraries(${EXE_NAME} PUBLIC loader PUBLIC opengl32 PUBLIC gdi32)
