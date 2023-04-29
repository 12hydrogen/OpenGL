#include "loader/textureLoader.hpp"

#include <iostream>

extern "C"
{
	#include <stb_image.h>
}



namespace opengl
{
	texture::texture(const string &filename, const string &type):
	textureType(convertMap.at(type))
	{
		auto pos = regTexture.find(filename);
		if (pos == regTexture.end())
		{

			int width, height, channels;
			stbi_set_flip_vertically_on_load(true);
			unsigned char *textureData = stbi_load(filename.c_str(), &width, &height, &channels, 0);
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
		else
			textureId = pos->second;
	}
	texture::texture(const string &name, const unsigned char *data, int channels, int width, int height, const string &type):
	textureType(convertMap.at(type))
	{
		auto pos = regTexture.find(name);
		if (pos == regTexture.end())
		{
			GLenum channelType;
			switch (channels)
			{
				case 1:
				channelType = GL_RED;
				break;
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

			glTexImage2D(textureType, 0, channelType, width, height, 0, channelType, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(textureType);
		}
		else
			textureId = pos->second;
	}
	texture::texture(GLuint id):
	textureId(id)
	{}
	texture::~texture()
	{
		auto end = regTexture.end();
		for (auto cur = regTexture.begin(); cur != end; ++cur)
		{
			if (cur->second == textureId)
			{
				glDeleteTextures(1, &textureId);
				regTexture.erase(cur);
				return;
			}
		}
	}
	void texture::useTexture() const
	{
		glBindTexture(textureType, textureId);
	}

	map<string, GLenum> texture::convertMap = {
		{"2d", GL_TEXTURE_2D},
		{"3d", GL_TEXTURE_2D}
	};
	map<string, GLuint> texture::regTexture;
}
