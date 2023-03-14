#pragma once
#include "loader.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include <string>
#include <map>

using namespace std;

class LOADER_SIGN texture
{
private:
	static map<string, GLenum> convertMap;

	GLuint textureId;
	GLenum textureType;
public:
	texture() = delete;
	texture(const string &name, const string &type);

	void useTexture() const;
};
