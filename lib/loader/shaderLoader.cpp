#include "loader/shaderLoader.hpp"

#include <iostream>
#include <fstream>

namespace opengl
{
	shader::shader(const string &str, GLenum shaderType):
	shaderType(shaderType)
	{
		string code;
		if (str.find_first_of(" \n\t") == str.npos)
		{
			ifstream file(str);
			code.assign(istreambuf_iterator<GLchar>(file), istreambuf_iterator<GLchar>());
			file.close();
		}
		else
		{
			code = str;
		}

		const GLchar *rawCode = code.c_str();
		shaderId = glCreateShader(shaderType);
		glShaderSource(shaderId, 1, &rawCode, NULL);
		glCompileShader(shaderId);

		GLint successCode;
		GLchar status[ERROR_LOG_BUFFER_SIZE] = {};
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &successCode);
		if (!successCode)
		{
			glGetShaderInfoLog(shaderId, ERROR_LOG_BUFFER_SIZE, NULL, status);
			throw error("Error compiling.", status);
		}
	}
	shader::~shader()
	{
		glDeleteShader(shaderId);
	}

	uniformSetter::uniformSetter():
	position(0)
	{}
	uniformSetter::uniformSetter(GLint position):
	position(position)
	{}
	void uniformSetter::operator=(const initializer_list<float> &vector) const
	{
		auto beg = vector.begin();
		switch(vector.size())
		{
			case 1:
			glUniform1f(position, beg[0]);
			break;
			case 2:
			glUniform2f(position, beg[0], beg[1]);
			break;
			case 3:
			glUniform3f(position, beg[0], beg[1], beg[2]);
			break;
			case 4:
			glUniform4f(position, beg[0], beg[1], beg[2], beg[3]);
			break;
			default:
			throw error("Invalid argument.");
		}
	}
	void uniformSetter::operator=(const vector<float> &vector) const
	{
		switch(vector.size())
		{
			case 1:
			glUniform1f(position, vector[0]);
			break;
			case 2:
			glUniform2f(position, vector[0], vector[1]);
			break;
			case 3:
			glUniform3f(position, vector[0], vector[1], vector[2]);
			break;
			case 4:
			glUniform4f(position, vector[0], vector[1], vector[2], vector[3]);
			break;
			default:
			throw error("Invalid argument.");
		}
	}
	void uniformSetter::operator=(const glm::vec3 &vector) const
	{
		glUniform3f(position, vector.r, vector.g, vector.b);
	}
	void uniformSetter::operator=(const initializer_list<int> &vector) const
	{
		auto beg = vector.begin();
		switch(vector.size())
		{
			case 1:
			glUniform1i(position, beg[0]);
			break;
			case 2:
			glUniform2i(position, beg[0], beg[1]);
			break;
			case 3:
			glUniform3i(position, beg[0], beg[1], beg[2]);
			break;
			case 4:
			glUniform4i(position, beg[0], beg[1], beg[2], beg[3]);
			break;
			default:
			throw error("Invalid argument.");
		}
	}
	void uniformSetter::operator=(const initializer_list<bool> &vector) const
	{
		auto beg = vector.begin();
		switch(vector.size())
		{
			case 1:
			glUniform1i(position, (int)beg[0]);
			break;
			case 2:
			glUniform2i(position, (int)beg[0], (int)beg[1]);
			break;
			case 3:
			glUniform3i(position, (int)beg[0], (int)beg[1], (int)beg[2]);
			break;
			case 4:
			glUniform4i(position, (int)beg[0], (int)beg[1], (int)beg[2], (int)beg[3]);
			break;
			default:
			throw error("Invalid argument.");
		}
	}
	void uniformSetter::operator=(const glm::mat4 &mat) const
	{
		glUniformMatrix4fv(position, 1, GL_FALSE, glm::value_ptr(mat));
	}
	void uniformSetter::operator=(const glm::mat3 &mat) const
	{
		glUniformMatrix3fv(position, 1, GL_FALSE, glm::value_ptr(mat));
	}
	void uniformSetter::operator=(const initializer_list<any> &vector) const
	{
		auto beg = vector.begin();
		switch(vector.size())
		{
			case 1:
			{
				if (beg[0].type() == typeid(glm::mat4))
					glUniformMatrix4fv(position, 1, GL_FALSE, glm::value_ptr(any_cast<glm::mat4>(beg[0])));
				else if (beg[0].type() == typeid(float))
					glUniform1f(position, any_cast<float>(beg[0]));
				else if (beg[0].type() == typeid(int))
					glUniform1i(position, any_cast<int>(beg[0]));
				else if (beg[0].type() == typeid(bool))
					glUniform1i(position, any_cast<int>(beg[0]));
				else
					throw error("Invalid argument.");
			}
			break;
			case 2:
			{
				if (beg[0].type() == typeid(float))
					glUniform2f(position, any_cast<float>(beg[0]), any_cast<float>(beg[1]));
				else if (beg[0].type() == typeid(int))
					glUniform2i(position, any_cast<int>(beg[0]), any_cast<int>(beg[1]));
				else if (beg[0].type() == typeid(bool))
					glUniform2i(position, any_cast<int>(beg[0]), any_cast<int>(beg[1]));
				else
					throw error("Invalid argument.");
			}
			break;
			case 3:
			{
				if (beg[0].type() == typeid(float))
					glUniform3f(position, any_cast<float>(beg[0]), any_cast<float>(beg[1]), any_cast<float>(beg[2]));
				else if (beg[0].type() == typeid(int))
					glUniform3i(position, any_cast<int>(beg[0]), any_cast<int>(beg[1]), any_cast<int>(beg[2]));
				else if (beg[0].type() == typeid(bool))
					glUniform3i(position, any_cast<int>(beg[0]), any_cast<int>(beg[1]), any_cast<int>(beg[2]));
				else
					throw error("Invalid argument.");
			}
			break;
			case 4:
			{
				if (beg[0].type() == typeid(float))
					glUniform4f(position, any_cast<float>(beg[0]), any_cast<float>(beg[1]), any_cast<float>(beg[2]), any_cast<float>(beg[3]));
				else if (beg[0].type() == typeid(int))
					glUniform4i(position, any_cast<int>(beg[0]), any_cast<int>(beg[1]), any_cast<int>(beg[2]), any_cast<int>(beg[3]));
				else if (beg[0].type() == typeid(bool))
					glUniform4i(position, any_cast<int>(beg[0]), any_cast<int>(beg[1]), any_cast<int>(beg[2]), any_cast<int>(beg[3]));
				else
					throw error("Invalid argument.");
			}
			break;
			default:
			throw error("Invalid argument.");
		}
	}

	shaderProgram::shaderProgram()
	{}
	shaderProgram::shaderProgram(const string &vShader, const string &fShader)
	{
		auto pos = regProgram.find(vShader + fShader);
		if (pos == regProgram.end())
		{
			programId = glCreateProgram();

			shader vertexShader(vShader, GL_VERTEX_SHADER);
			shader fragmentShader(fShader, GL_FRAGMENT_SHADER);

			linkProgram(vertexShader, fragmentShader);
		}
		else
			programId = pos->second;
	}
	shaderProgram::shaderProgram(const json &jsonFile)
	{
		if (!jsonFile.is_object())
			throw error("JSON format error.");
		if (!(jsonFile.contains("vertex") && jsonFile.contains("fragment")))
			throw error("JSON format error.");
		if (!(jsonFile["vertex"].is_string() && jsonFile["fragment"].is_string()))
			throw error("Value type error.");
		auto pos = regProgram.find(jsonFile["vertex"].get<string>() + jsonFile["fragment"].get<string>());
		if (pos == regProgram.end())
		{
			programId = glCreateProgram();

			shader vertexShader(jsonFile["vertex"].get<string>(), GL_VERTEX_SHADER);
			shader fragmentShader(jsonFile["fragment"].get<string>(), GL_FRAGMENT_SHADER);

			linkProgram(vertexShader, fragmentShader);
		}
		else
			programId = pos->second;
	}
	shaderProgram::~shaderProgram()
	{
		//glDeleteProgram(programId);
	}
	void shaderProgram::linkProgram(shader &vShader, shader &fShader)
	{
		glAttachShader(programId, vShader.getShader());
		glAttachShader(programId, fShader.getShader());
		glLinkProgram(programId);

		GLint successCode;
		GLchar status[ERROR_LOG_BUFFER_SIZE] = {};
		glGetProgramiv(programId, GL_LINK_STATUS, &successCode);
		if (!successCode)
		{
			glGetProgramInfoLog(programId, ERROR_LOG_BUFFER_SIZE, NULL, status);
			throw error("Error linking.", status);
		}
	}
	void shaderProgram::useProgram() const
	{
		glUseProgram(programId);
	}
	uniformSetter& shaderProgram::operator[](const string &name)
	{
		if (setterList.find(name) == setterList.end())
		{
			setterList.insert(make_pair(name, uniformSetter(glGetUniformLocation(programId, name.c_str()))));
		}
		return setterList[name];
	}
	map<string, GLuint> shaderProgram::regProgram;
}
