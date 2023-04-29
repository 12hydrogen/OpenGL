#pragma once

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "glm/glm.hpp"

#include "gl.hpp"
#include "loader/textureLoader.hpp"

#include <vector>
#include <initializer_list>
#include <any>
#include <functional>

namespace opengl
{
	using namespace std;

	typedef struct __loader_data {
		glm::vec3 vertex;
		glm::vec3 normal;
		glm::vec2 texture;
	}vertexData;

	typedef GLuint indiceData;

	typedef struct __plain_model {
		vector<vertexData> vertices;
		vector<indiceData> indices;

		map<string, texture> textures;

		const GLfloat* rawVertex() const;
		size_t rawVertexSize() const;
		size_t vertexSize() const;

		const GLuint* rawIndice() const;
		size_t rawIndiceSize() const;
		size_t indiceSize() const;
	}plainModel;

	class scene: public vector<plainModel>
	{
	private:
		const static auto defaultPostprocess = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals;

		string directory;

		void convertor(const aiNode *node, const aiScene *scene);
	public:
		scene() = delete;
		scene(const string &filename);

		template <class T>
		vector<T>&& map(function<T&& (const plainModel&)> hook)
		{
			vector<T> *ret = new vector<T>();
			ret->reserve(this->size());
			for (auto model : *this)
			{
				ret->emplace_back(hook(model));
			}
			return move(*ret);
		}
	};
}
