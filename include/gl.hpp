#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "error.hpp"

#if defined(EXPORT)
#define DLL_SIGN __declspec(dllexport)
#elif defined(IMPORT)
#define DLL_SIGN __declspec(dllimport)
#else
#define DLL_SIGN
#endif

namespace opengl
{
    void glErrorAssert();
}
