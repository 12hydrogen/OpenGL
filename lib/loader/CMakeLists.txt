add_definitions(-g -Wall -Werror -static)

include_directories("${OpenGL-Test-Program_SOURCE_DIR}/include")
link_directories("${OpenGL-Test-Program_SOURCE_DIR}/lib")

add_library(loader SHARED "arrayLoader.cpp" "shaderLoader.cpp" "textureLoader.cpp" "modelLoader.cpp" "gl.cpp")
target_link_libraries(loader PUBLIC glad PUBLIC assimp)
