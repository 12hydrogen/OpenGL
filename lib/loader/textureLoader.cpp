#include "textureLoader.hpp"

#include <iostream>

extern "C"
{
	#include <stb_image.h>
}



namespace opengl
{
	texture::texture(const string &name, const string &type):
	textureType(convertMap.at(type))
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char *textureData = stbi_load(name.c_str(), &width, &height, &channels, 0);
		if (textureData == NULL)
		{
			throw error("Texture image load failed.");
		}
		GLenum channelType;
		switch (channels)
		{
			case 3:
			channelType = GL_RGB;
			break;
			case 4:
			channelType = GL_RGBA;
			break;
			default:
			throw error("Channel type cannot be set automatically.");
		}

		glGenTextures(1, &textureId);
		glBindTexture(textureType, textureId);

		glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(textureType, 0, channelType, width, height, 0, channelType, GL_UNSIGNED_BYTE, textureData);
		glGenerateMipmap(textureType);

		stbi_image_free(textureData);
	}
	void texture::useTexture() const
	{
		glBindTexture(textureType, textureId);
	}

	map<string, GLenum> texture::convertMap = {
		{"2d", GL_TEXTURE_2D},
		{"3d", GL_TEXTURE_2D}
	};
}
