#pragma once

#include "loader/shaderLoader.hpp"
#include "loader/textureLoader.hpp"
#include "loader/modelLoader.hpp"

#include <vector>
#include <string>
#include <fstream>

#include "nlohmann/json.hpp"

#include "glm/glm.hpp"

namespace opengl
{
	using json = nlohmann::json;

	using namespace std;

	// Interface class for vertexArray and indiceArray
	template <typename T>
	class DLL_SIGN baseArray
	{
	protected:
		GLuint bufferObject;
		vector<T> data;

		virtual void loadData(const json &arrayFile) = 0;

		const T* getData();
	public:
		baseArray(){}
		baseArray(const vector<T> &data);
		virtual ~baseArray(){}

		GLuint getLength() const;
		GLuint getSize() const;

		virtual void bindBuffer() const = 0;
		virtual void genBuffer(GLenum usage) = 0;
		virtual void setVertexPointer(GLenum normalize) const = 0;
	};

	class DLL_SIGN vertexArray: public baseArray<GLfloat>
	{
	private:
		vector<GLuint> depth;

		GLuint verticeCount;
		GLuint lengthPerCount;

		virtual void loadData(const json &arrayFile);
	public:
		vertexArray(const vector<GLfloat> &rawData, initializer_list<GLuint> &&depth = {3, 3, 2}):
		baseArray(rawData), depth(depth)
		{
			lengthPerCount = 0;
			for (auto len : this->depth)
			{
				lengthPerCount += len;
			}
			verticeCount = this->depth.size() / lengthPerCount;
		}

		vertexArray(const string &filename);
		vertexArray(ifstream &arrayFile);
		vertexArray(const json &arrayFile);
		virtual ~vertexArray(){}

		virtual void bindBuffer() const;
		virtual void genBuffer(GLenum usage);
		virtual void setVertexPointer(GLenum normalize) const;
	};

	class DLL_SIGN indiceArray: public baseArray<GLuint>
	{
	private:
		const static DLL_SIGN map<string, GLenum> convertMap;

		GLenum primitive;

		virtual void loadData(const json &arrayFile);
	public:
		indiceArray(const vector<GLuint> &rawData, GLenum primitive = GL_TRIANGLES):
		baseArray(rawData), primitive(primitive){}

		indiceArray(const string &filename);
		indiceArray(ifstream &arrayFile);
		indiceArray(const json &arrayFile);
		virtual ~indiceArray(){}

		virtual void bindBuffer() const;
		virtual void genBuffer(GLenum usage);
		virtual void setVertexPointer(GLenum normalize) const;

		GLenum getPrimitive() const;
	};

	class DLL_SIGN singleObject
	{
	private:
		GLuint arrayObject;
		vertexArray *vArray;
		indiceArray *iArray;
		shaderProgram *sProgram;
		map<string, texture> *textureList;

		void genObject(const json &jsonObject);

	public:
		singleObject();
		singleObject(const string &filename);
		singleObject(ifstream &file);
		singleObject(const json &jsonObject);
		~singleObject();

		shaderProgram& getShaderProgram();
		const shaderProgram& getShaderProgram() const;
		const map<string, texture>& getTextureList() const;

		void genObjectBuffer(GLenum usage = GL_STATIC_DRAW, GLenum normalize = GL_FALSE);

		void draw() const;

		friend class objectArray;
	};

	typedef glm::mat4 (*transformCallback)(void*) ;

	class DLL_SIGN objectArray
	{
	public:
		typedef struct __object_usage {
			glm::vec3 model;

			glm::vec3 *color;
			struct __material {
				glm::vec3 ambient;
				glm::vec3 diffuse;
				glm::vec3 specular;
				float shininess;
			}material;

			glm::vec3 rotateAxis;
			GLfloat rotateDegree;

			transformCallback callback;

			__object_usage():
			color(NULL), callback(NULL){}
		}objectUsage;

		typedef struct __light_usage {
			string name;
			GLuint id;

			glm::vec3 pos;
			glm::vec3 color;
			glm::vec3 strength;
			glm::vec3 attenuation;

			glm::vec3 direction;
			float cutoff;
			float outerCutoff;

			__light_usage(){}
			__light_usage(const string &name, GLuint id, const glm::vec3 &pos):
			name(name), id(id), pos(pos), cutoff(0.0f), outerCutoff(0.0f){}
		}lightUsage;

	private:
		map<string, vector<singleObject>> defination;
		map<string, vector<objectUsage>> usage;

		map<string, lightUsage> lightSource;

		void genArray(const json &jsonObject, bool gen);

		void genDefination(const json &jsonObject, const string &name);
		static singleObject&& genObject(const plainModel &model);
		void genUsage(const json &jsonObject, const string &name);
		objectUsage genUsageAttr(const json &jsonObject, const string &name, GLuint id);
	public:
		objectArray(const string &filename, bool gen = true);
		objectArray(const char *filename, bool gen = true);
		objectArray(ifstream &file, bool gen = true);
		objectArray(const json &jsonObject, bool gen = true);
		~objectArray();

		auto& operator[](const string &str);
		const auto& operator[](const string &str) const;

		void draw(const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &viewPos, const glm::vec3 &viewFacing, void *globalInfo);

		map<string, vector<singleObject>>& getDefination();
		const map<string, vector<singleObject>>& getDefination() const;
		map<string, vector<objectUsage>>& getUsage();
		const map<string, vector<objectUsage>>& getUsage() const;
	};
}
