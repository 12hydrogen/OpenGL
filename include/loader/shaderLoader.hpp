#pragma once
#include "gl.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "nlohmann/json.hpp"

#include <string>
#include <initializer_list>
#include <map>
#include <variant>

namespace opengl
{
	using json = nlohmann::json;

	#define ERROR_LOG_BUFFER_SIZE 5120

	using namespace std;

	class DLL_SIGN shader
	{
	private:
		GLenum shaderType;
		GLuint shaderId;
	public:
		shader(const string &filename, GLenum shaderType);
		~shader();

		GLuint getShader() const
		{
			return shaderId;
		}
	};

	class DLL_SIGN uniformSetter
	{
	private:
		GLint position;
	public:
		uniformSetter();
		explicit uniformSetter(GLint position);

		void operator=(const vector<float> &vector) const;
		void operator=(const glm::vec3 &vector) const;

		void operator=(const initializer_list<float> &vector) const;
		void operator=(const initializer_list<int> &vector) const;
		void operator=(const initializer_list<bool> &vector) const;
		void operator=(const glm::mat4 &mat) const;
		void operator=(const glm::mat3 &mat) const;

		void operator=(const initializer_list<any> &vector) const;
	};

	class DLL_SIGN shaderProgram
	{
	private:
		static map<string, GLuint> regProgram;

		GLuint programId;

		map<string, uniformSetter> setterList;

		void linkProgram(shader &vShader, shader &fShader);
	public:
		shaderProgram();
		shaderProgram(const string &vShader, const string &fShader);
		shaderProgram(const json &jsonFile);
		~shaderProgram();

		void useProgram() const;

		GLuint getProgram() const
		{
			return programId;
		}

		uniformSetter& operator[](const string &name);
	};
}
