#pragma once
#include "gl.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include <string>
#include <map>

namespace opengl
{
	using namespace std;

	class DLL_SIGN texture
	{
	private:
		static map<string, GLuint> regTexture;
		static map<string, GLenum> convertMap;

		GLuint textureId;
		GLenum textureType;

		texture(GLuint id);
	public:
		texture() = delete;
		texture(const string &filename, const string &type = "2d");
		texture(const string &name, const unsigned char *data, int channels, int width, int height, const string &type);
		~texture();

		void useTexture() const;
	};
}
