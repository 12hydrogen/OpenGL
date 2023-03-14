#include "arrayLoader.hpp"

#include <iostream>

#include <cstdlib>
#include <cstring>

// baseArray
template <typename T>
baseArray<T>::baseArray()
{}
template <typename T>
baseArray<T>::~baseArray()
{}

template <typename T>
const T* baseArray<T>::getData()
{
	return rawData.data();
}
template <typename T>
GLuint baseArray<T>::getLength() const
{
	return rawData.size();
}
template <typename T>
GLuint baseArray<T>::getSize() const
{
	return rawData.size() * sizeof(T);
}

template class baseArray<GLfloat>;
template class baseArray<GLuint>;

// vertexArray
vertexArray::vertexArray(const string &filename):
verticeCount(0), lengthPerCount(0)
{
	ifstream file(filename);
	json jsonFile = json::parse(file);
	file.close();
	loadData(jsonFile);
}
vertexArray::vertexArray(ifstream &arrayFile):
verticeCount(0), lengthPerCount(0)
{
	json jsonFile = json::parse(arrayFile);
	loadData(jsonFile);
}
vertexArray::vertexArray(const json &arrayFile):
verticeCount(0), lengthPerCount(0)
{
	loadData(arrayFile);
}

void vertexArray::genBuffer(GLenum usage)
{
	glGenBuffers(1, &bufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject);
	const GLfloat *tempData = this->getData();
	glBufferData(GL_ARRAY_BUFFER, this->getSize(), tempData, usage);
}
void vertexArray::setVertexPointer(GLenum normalize) const
{
	GLuint offset = 0;
	for (GLuint index = 0; index < depth.size(); index++)
	{
		glVertexAttribPointer(index, depth[index], GL_FLOAT, normalize, lengthPerCount * sizeof(GLfloat), (void*)(offset * sizeof(GLfloat)));
		glEnableVertexAttribArray(index);
		offset += depth[index];
	}
}

void vertexArray::loadData(const json &arrayFile)
{
	if (!(arrayFile.is_object() && arrayFile.contains("value") && arrayFile.contains("structure")))
		throw "JSON format error.";
	if (!arrayFile["value"][0].is_number_float())
		throw "Value type error.";

	rawData = arrayFile["value"].get<vector<GLfloat>>();
	depth = arrayFile["structure"].get<vector<GLuint>>();

	for (auto i : depth)
	{
		lengthPerCount += i;
	}

	if (rawData.size() % lengthPerCount != 0)
		throw "Value incomplete.";

	verticeCount = rawData.size() / lengthPerCount;

}

void vertexArray::bindBuffer() const
{
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject);
}

// indiceArray
indiceArray::indiceArray(const string &filename)
{
	ifstream file(filename);
	json jsonFile = json::parse(file);
	file.close();
	loadData(jsonFile);
}
indiceArray::indiceArray(ifstream &arrayFile)
{
	json jsonFile = json::parse(arrayFile);
	loadData(jsonFile);
}
indiceArray::indiceArray(const json &arrayFile)
{
	loadData(arrayFile);
}

void indiceArray::genBuffer(GLenum usage)
{
	glGenBuffers(1, &bufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObject);
	const GLuint *tempData = this->getData();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->getSize(), tempData, usage);
}
void indiceArray::setVertexPointer(GLenum normalize) const
{
	throw "Unable to set vertex pointer of element buffer object.";
}

void indiceArray::loadData(const json &arrayFile)
{
	if (!(arrayFile.is_object() && arrayFile.contains("value") && arrayFile.contains("primitive")))
		throw "JSON format error.";
	if (!arrayFile["value"][0].is_number_unsigned())
		throw "Value type error.";

	rawData = arrayFile["value"].get<vector<GLuint>>();

	string primitiveStr = arrayFile["primitive"];

	if (convertMap.count(primitiveStr) == 0)
		throw "Primitive not supported.";
	else
		primitive = convertMap.at(primitiveStr);
}
void indiceArray::bindBuffer() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObject);
}
GLenum indiceArray::getPrimitive() const
{
	return primitive;
}

// singleObject
singleObject::singleObject()
{}
singleObject::singleObject(const string &filename)
{
	ifstream file(filename);
	json jsonFile = json::parse(file);
	file.close();
	genObject(jsonFile);
}
singleObject::singleObject(ifstream &file)
{
	json jsonFile = json::parse(file);
	genObject(jsonFile);
}
singleObject::singleObject(const json &file)
{
	genObject(file);
}
singleObject::~singleObject()
{}

shaderProgram& singleObject::getShaderProgram()
{
	return sProgram;
}
const map<string, texture>& singleObject::getTextureList() const
{
	return textureList;
}

void singleObject::genObjectBuffer(GLenum usage/* = GL_STATIC_DRAW*/, GLenum normalize/* = GL_FALSE*/)
{
	// Gen VAO
	glGenVertexArrays(1, &arrayObject);
	glBindVertexArray(arrayObject);
	// Gen VBO
	vArray.genBuffer(usage);
	// Gen EBO
	iArray.genBuffer(usage);
	// Set VP
	vArray.setVertexPointer(normalize);
}

void singleObject::draw() const
{
	glBindVertexArray(arrayObject);
	glDrawElements(iArray.getPrimitive(), iArray.getLength(), GL_UNSIGNED_INT, 0);
}

void singleObject::genObject(const json &file)
{
	if (!(file.is_object() && file.contains("vertex") && file.contains("indice") && file.contains("shader")))
		throw "JSON format error.";
	vArray = vertexArray(file["vertex"]);
	iArray = indiceArray(file["indice"]);
	sProgram = shaderProgram(file["shader"]);

	if (file.contains("texture"))
	{
		if (!file["texture"].is_array())
			throw "JSON format error.";
		for (auto &singleTexture : file["texture"])
		{
			if (!singleTexture.is_object())
				throw "JSON format error.";
			if (!(singleTexture.contains("name") && singleTexture.contains("file") && singleTexture.contains("type")))
				throw "JSON format error.";
			if (!(singleTexture["file"].is_string() && singleTexture["type"].is_string()))
				throw "Value type error.";
			textureList.insert_or_assign(singleTexture["name"].get<string>(), texture(singleTexture["file"].get<string>(), singleTexture["type"].get<string>()));
		}
	}
}

// objectArray
objectArray::objectArray(const char *filename, bool gen/* = true*/)
{
	// This function encounters problems, probably because of relative path
	ifstream file(filename);
	json jsonFile = json::parse(file);
	file.close();
	genArray(jsonFile, gen);
}
objectArray::objectArray(const string &filename, bool gen/* = true*/)
{
	ifstream file(filename);
	json jsonFile = json::parse(file);
	file.close();
	genArray(jsonFile, gen);
}
objectArray::objectArray(ifstream &file, bool gen/* = true*/)
{
	json jsonFile = json::parse(file);
	genArray(jsonFile, gen);
}
objectArray::objectArray(const json &file, bool gen/* = true*/)
{
	genArray(file, gen);
}

void objectArray::genArray(const json &file, bool gen)
{
	if (!file.is_array())
		throw "JSON format error.";

	for (GLuint i = 0; i < file.size(); i++)
	{
		// Basic confirm, json object with "type" and "name"
		if (file[i].contains("type") && file[i].contains("name"))
		{
			// "type" is string and "name" is string
			if (file[i]["type"].is_string() && file[i]["name"].is_string())
			{
				string type = file[i]["type"].get<string>();
				string name = file[i]["name"].get<string>();
				// This is a defination to a object
				// Defines the vertices of object
				// Contains value of VBO and EBO
				if (type.compare("defination") == 0)
				{
					// Create a singleObject for further resolve
					defination.insert(make_pair(name, singleObject(file[i])));
				}
				// This is a usage to a object
				// Contains coordinates to transform object to
				else if (type.compare("usage") == 0 && file[i].contains("offset") && file[i].contains("model"))
				{
					if (file[i]["model"].is_string())
					{
						// Set the model mat name
						usage[name].first = file[i]["model"].get<string>();
					}
					// Coordinates array
					if (file[i]["offset"].is_array())
					{
						// Single coordinate
						if (file[i]["offset"][0].is_array())
						{
							// Coordinate in float
							if (file[i]["offset"][0][0].is_number_float())
							{
								// Insert if this object has not been used
								if (usage.count(name) == 0)
									usage.insert(make_pair(name, pair<string, vector<vector<GLfloat>>>()));
								// Reserve space for performance
								usage[name].second.reserve(usage[name].second.size() + file[i]["offset"].size());
								for (auto &offsetArray : file[i]["offset"])
								{
									// Only accept 7 float, x, y, z, rotate, rotateX, rotateY, rotateZ
									if (offsetArray.size() != 7)
										continue;
									// Direct convert
									usage[name].second.emplace_back(offsetArray.get<vector<GLfloat>>());
								}
							}
						}
					}
				}
			}
		}
	}
	if (gen)
	{
		for (auto &i : defination)
		{
			i.second.genObjectBuffer();
		}
	}
}

void objectArray::draw(const glm::mat4 &preModel, const glm::mat4 &view, const glm::mat4 &projection)
{
	GLenum textureUnit = GL_TEXTURE0;
	for (auto drawList: usage)
	{
		auto &sObj = defination[drawList.first];
		auto &sProgram = sObj.getShaderProgram();
		sProgram.useProgram();
		sProgram["view"] = {view};
		sProgram["projection"] = {projection};
		for (auto &singleTexture : sObj.getTextureList())
		{
			if (textureUnit > GL_TEXTURE16)
				throw "Too many texture unit.";
			glActiveTexture(textureUnit);
			singleTexture.second.useTexture();
			sProgram[singleTexture.first] = {(int)(textureUnit - GL_TEXTURE0)};
			textureUnit++;
		}
		if (defination.count(drawList.first) == 0)
			continue;
		for (auto drawSingle: drawList.second.second)
		{
			glm::mat4 model(1.0f);
			model = glm::translate(model, glm::vec3(drawSingle[0], drawSingle[1], drawSingle[2]));
			model = glm::rotate(model, glm::radians(drawSingle[3]), glm::vec3(drawSingle[4], drawSingle[5], drawSingle[6]));
			model *= preModel;
			sProgram[drawList.second.first] = {model};
			sObj.draw();
		}
	}
}

singleObject& objectArray::operator[](const string &str)
{
	return defination[str];
}
const singleObject& objectArray::operator[](const string &str) const
{
	return defination.at(str);
}

map<string, singleObject>& objectArray::getDefination()
{
	return defination;
}
const map<string, singleObject>& objectArray::getDefination() const
{
	return defination;
}

const map<string, GLenum> indiceArray::convertMap = {
	{"point", GL_POINTS},
	{"line", GL_LINES},
	{"continue_line", GL_LINE_STRIP},
	{"loop", GL_LINE_LOOP},
	{"triangle", GL_TRIANGLES},
	{"continue_triangle", GL_TRIANGLE_STRIP},
	{"fan_triangle", GL_TRIANGLE_FAN},
};
