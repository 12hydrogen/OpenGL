#include "gl.hpp"

namespace opengl
{
    using namespace std;
    void glErrorAssert()
    {
        int errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            throw error(to_string(errorCode));
        }
    }
}
