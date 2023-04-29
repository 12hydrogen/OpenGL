add_definitions(-g -Wall -Werror -static)

include_directories("${OpenGL-Test-Program_SOURCE_DIR}/include")
link_directories("${OpenGL-Test-Program_SOURCE_DIR}/lib")

add_library(glad SHARED "glad.c")

add_library(interface STATIC "interface.cpp")
target_link_libraries(interface PUBLIC loader PUBLIC glad PUBLIC glfw3)

add_subdirectory("loader")
