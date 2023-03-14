add_definitions(-g -Wall -Werror -DSTATIC)

include_directories("${OpenGL-Test-Program_SOURCE_DIR}/include")
link_directories("${OpenGL-Test-Program_SOURCE_DIR}/lib")

link_libraries(glfw3 m)
add_library(loader STATIC "arrayLoader.cpp" "shaderLoader.cpp" "textureLoader.cpp")
