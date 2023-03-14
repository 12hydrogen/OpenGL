#include "shaderLoader.hpp"

#include <iostream>
#include <fstream>

shader::shader(const string &str, GLenum shaderType):
shaderType(shaderType)
{
	string code;
	if (str.find_first_of(" \n\t") == str.npos)
	{
		ifstream file(str);
		code.assign(istreambuf_iterator<char>(file), istreambuf_iterator<char>());
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
		throw status;
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
void uniformSetter::operator=(const initializer_list<float> vector) const
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
		throw "Invalid argument.";
	}
}
void uniformSetter::operator=(const initializer_list<int> vector) const
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
		throw "Invalid argument.";
	}
}
void uniformSetter::operator=(const initializer_list<bool> vector) const
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
		throw "Invalid argument.";
	}
}
void uniformSetter::operator=(const initializer_list<glm::mat4> vector) const
{
	auto beg = vector.begin();
	switch(vector.size())
	{
		case 1:
		glUniformMatrix4fv(position, 1, GL_FALSE, glm::value_ptr(beg[0]));
		break;
		default:
		throw "Invalid argument.";
	}
}

shaderProgram::shaderProgram()
{}
shaderProgram::shaderProgram(const string &vShader, const string &fShader)
{
	programId = glCreateProgram();

	shader vertexShader(vShader, GL_VERTEX_SHADER);
	shader fragmentShader(fShader, GL_FRAGMENT_SHADER);

	linkProgram(vertexShader, fragmentShader);
}
shaderProgram::shaderProgram(const json &jsonFile)
{
	if (!jsonFile.is_object())
		throw "JSON format error.";
	if (!(jsonFile.contains("vertex") && jsonFile.contains("fragment")))
		throw "JSON format error.";
	if (!(jsonFile["vertex"].is_string() && jsonFile["fragment"].is_string()))
		throw "Value type error.";

	programId = glCreateProgram();

	shader vertexShader(jsonFile["vertex"].get<string>(), GL_VERTEX_SHADER);
	shader fragmentShader(jsonFile["fragment"].get<string>(), GL_FRAGMENT_SHADER);

	linkProgram(vertexShader, fragmentShader);
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
		throw status;
	}
}
void shaderProgram::useProgram() const
{
	glUseProgram(programId);
}
const uniformSetter& shaderProgram::operator[](const string &name)
{
	if (setterList.find(name) == setterList.end())
	{
		setterList.insert(make_pair(name, uniformSetter(glGetUniformLocation(programId, name.c_str()))));
	}
	return setterList[name];
}
