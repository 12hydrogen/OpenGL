#include "loader/modelLoader.hpp"

namespace opengl
{
	const GLfloat *plainModel::rawVertex() const
	{
		return (const GLfloat *)vertices.data();
	}
	size_t plainModel::rawVertexSize() const
	{
		return vertices.size() * (sizeof(vertexData) / sizeof(GLfloat));
	}
	size_t plainModel::vertexSize() const
	{
		return vertices.size();
	}

	const GLuint* plainModel::rawIndice() const
	{
		return (const GLuint *)indices.data();
	}
	size_t plainModel::rawIndiceSize() const
	{
		return indices.size() * (sizeof(indiceData) / sizeof(GLuint));
	}
	size_t plainModel::indiceSize() const
	{
		return indices.size();
	}

	void scene::convertor(const aiNode *node, const aiScene *scene)
	{
		plainModel *model  = new plainModel();
		vertexData *entity = new vertexData();
		for (size_t meshCount = 0; meshCount < node->mNumMeshes; meshCount++)
		{
			aiMesh *mesh = scene->mMeshes[node->mMeshes[meshCount]];
			model->vertices.reserve(model->vertices.size() + mesh->mNumVertices);

			for (size_t vertexCount = 0; vertexCount < mesh->mNumVertices; vertexCount++)
			{
				entity->vertex.x = mesh->mVertices[vertexCount].x;
				entity->vertex.y = mesh->mVertices[vertexCount].y;
				entity->vertex.z = mesh->mVertices[vertexCount].z;

				entity->normal.x = mesh->mNormals[vertexCount].x;
				entity->normal.y = mesh->mNormals[vertexCount].y;
				entity->normal.z = mesh->mNormals[vertexCount].z;

				if (mesh->mTextureCoords[0])
				{
					entity->texture.x = mesh->mTextureCoords[0][vertexCount].x;
					entity->texture.y = mesh->mTextureCoords[0][vertexCount].y;
				}

				model->vertices.emplace_back(*entity);
			}

			model->indices.reserve(model->indices.size() + mesh->mNumFaces * mesh->mFaces[0].mNumIndices);

			for (size_t faceCount = 0; faceCount < mesh->mNumFaces; faceCount++)
			{
				aiFace face = mesh->mFaces[faceCount];
				for (size_t indiceCount = 0; indiceCount < face.mNumIndices; indiceCount++)
				{
					model->indices.emplace_back(face.mIndices[indiceCount]);
				}
			}

			if (mesh->mMaterialIndex >= 0)
			{
				aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
				for (size_t materialCount = 0; materialCount < material->GetTextureCount(aiTextureType_DIFFUSE); materialCount++)
				{
					aiString name;
					material->GetTexture(aiTextureType_DIFFUSE, materialCount, &name);
					string nameCleared("diffuseTexture_");
					nameCleared += to_string(materialCount);
					model->textures.emplace(make_pair(nameCleared, texture(directory + name.C_Str(), "2d")));
				}
				for (size_t materialCount = 0; materialCount < material->GetTextureCount(aiTextureType_SPECULAR); materialCount++)
				{
					aiString name;
					material->GetTexture(aiTextureType_SPECULAR, materialCount, &name);

					string nameCleared("specularTexture_");
					nameCleared += to_string(materialCount);

					model->textures.emplace(make_pair(nameCleared, texture(directory + name.C_Str(), "2d")));
				}
			}
		}

		delete entity;
		if (node->mNumMeshes != 0)
			this->emplace_back(*model);
		delete model;

		for(unsigned int i = 0; i < node->mNumChildren; i++)
		{
			convertor(node->mChildren[i], scene);
		}

		return;
	}

	scene::scene(const string &filename)
	{
		Assimp::Importer import;
		auto scene = import.ReadFile(filename, defaultPostprocess);
		if (!scene || (scene->mFlags && AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
		{
			throw error("Model load failed.", import.GetErrorString());
		}
		directory = filename.substr(0, filename.find_last_of('/') + 1);
		convertor(scene->mRootNode, scene);
	}
}
