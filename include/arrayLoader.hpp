#pragma once

#include "shaderLoader.hpp"
#include "textureLoader.hpp"

#include <vector>
#include <string>
#include <fstream>

#include <nlohmann/json.hpp>

#include <glm/glm.hpp>

using json = nlohmann::json;

using namespace std;

// Interface class for vertexArray and indiceArray
template <typename T>
class LOADER_SIGN baseArray
{
protected:
	GLuint bufferObject;
	vector<T> rawData;

	baseArray();

	virtual void loadData(const json &arrayFile) = 0;

	const T* getData();
public:
	~baseArray();

	GLuint getLength() const;
	GLuint getSize() const;

	virtual void bindBuffer() const = 0;
	virtual void genBuffer(GLenum usage) = 0;
	virtual void setVertexPointer(GLenum normalize) const = 0;
};

class LOADER_SIGN vertexArray: public baseArray<float>
{
private:
	vector<GLuint> depth;

	GLuint verticeCount;
	GLuint lengthPerCount;

	virtual void loadData(const json &arrayFile);
public:
	vertexArray(){}
	vertexArray(const string &filename);
	vertexArray(ifstream &arrayFile);
	vertexArray(const json &arrayFile);

	virtual void bindBuffer() const;
	virtual void genBuffer(GLenum usage);
	virtual void setVertexPointer(GLenum normalize) const;
};

class LOADER_SIGN indiceArray: public baseArray<unsigned int>
{
private:
	const static map<string, GLenum> convertMap;

	GLenum primitive;

	virtual void loadData(const json &arrayFile);
public:
	indiceArray(){}
	indiceArray(const string &filename);
	indiceArray(ifstream &arrayFile);
	indiceArray(const json &arrayFile);

	virtual void bindBuffer() const;
	virtual void genBuffer(GLenum usage);
	virtual void setVertexPointer(GLenum normalize) const;

	GLenum getPrimitive() const;
};

class LOADER_SIGN singleObject
{
private:
	GLuint arrayObject;
	vertexArray vArray;
	indiceArray iArray;
	shaderProgram sProgram;
	map<string, texture> textureList;

	void genObject(const json &file);

public:
	singleObject();
	singleObject(const string &filename);
	singleObject(ifstream &file);
	singleObject(const json &file);
	~singleObject();

	shaderProgram& getShaderProgram();
	const map<string, texture>& getTextureList() const;

	void genObjectBuffer(GLenum usage = GL_STATIC_DRAW, GLenum normalize = GL_FALSE);

	void draw() const;
};

class LOADER_SIGN objectArray
{
private:
	map<string, singleObject> defination;
	// name of object, name of model mat, model mats
	map<string, pair<string, vector<vector<GLfloat>>>> usage;

	void genArray(const json &file, bool gen);
public:
	objectArray(const string &filename, bool gen = true);
	objectArray(const char *filename, bool gen = true);
	objectArray(ifstream &file, bool gen = true);
	objectArray(const json &file, bool gen = true);

	singleObject& operator[](const string &str);
	const singleObject& operator[](const string &str) const;

	void draw(const glm::mat4 &preModel, const glm::mat4 &view, const glm::mat4 &projection);

	map<string, singleObject>& getDefination();
	const map<string, singleObject>& getDefination() const;
};
