#include "loader/arrayLoader.hpp"
#include "loader/modelLoader.hpp"

#include <iostream>

#include <cstdlib>
#include <cstring>
namespace opengl
{
	// baseArray
	template <typename T>
	baseArray<T>::baseArray(const vector<T> &data):
	data(data){}

	template <typename T>
	const T* baseArray<T>::getData()
	{
		return data.data();
	}
	template <typename T>
	GLuint baseArray<T>::getLength() const
	{
		return data.size();
	}
	template <typename T>
	GLuint baseArray<T>::getSize() const
	{
		return data.size() * sizeof(T);
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
			throw error("JSON format error.");
		if (!arrayFile["value"][0].is_number_float())
			throw error("Value type error.");

		data = arrayFile["value"].get<vector<GLfloat>>();
		depth = arrayFile["structure"].get<vector<GLuint>>();

		for (auto i : depth)
		{
			lengthPerCount += i;
		}

		if (data.size() % lengthPerCount != 0)
			throw error("Value incomplete.");

		verticeCount = data.size() / lengthPerCount;

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
		throw error("Unable to set vertex pointer of element buffer object.");
	}

	void indiceArray::loadData(const json &arrayFile)
	{
		if (!(arrayFile.is_object() && arrayFile.contains("value") && arrayFile.contains("primitive")))
			throw error("JSON format error.");
		if (!arrayFile["value"][0].is_number_unsigned())
			throw error("Value type error.");

		data = arrayFile["value"].get<vector<GLuint>>();

		string primitiveStr = arrayFile["primitive"];

		if (convertMap.count(primitiveStr) == 0)
			throw error("Primitive not supported.");
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
	singleObject::singleObject(const json &jsonObject)
	{
		genObject(jsonObject);
	}
	singleObject::~singleObject()
	{}

	shaderProgram& singleObject::getShaderProgram()
	{
		return *sProgram;
	}
	const shaderProgram& singleObject::getShaderProgram() const
	{
		return *sProgram;
	}
	const map<string, texture>& singleObject::getTextureList() const
	{
		return *textureList;
	}

	void singleObject::genObjectBuffer(GLenum usage/* = GL_STATIC_DRAW*/, GLenum normalize/* = GL_FALSE*/)
	{
		// Gen VAO
		glGenVertexArrays(1, &arrayObject);
		glBindVertexArray(arrayObject);
		// Gen VBO
		vArray->genBuffer(usage);
		// Gen EBO
		iArray->genBuffer(usage);
		// Set VP
		vArray->setVertexPointer(normalize);
		glBindVertexArray(0);
	}

	void singleObject::draw() const
	{
		glBindVertexArray(arrayObject);
		glDrawElements(iArray->getPrimitive(), iArray->getLength(), GL_UNSIGNED_INT, 0);
	}

	void singleObject::genObject(const json &jsonObject)
	{
		if (!(jsonObject.is_object() && jsonObject.contains("vertex") && jsonObject.contains("indice") && jsonObject.contains("shader")))
			throw error("JSON format error.");
		vArray = new vertexArray(jsonObject["vertex"]);
		iArray = new indiceArray(jsonObject["indice"]);
		sProgram = new shaderProgram(jsonObject["shader"]);
		textureList = new map<string, texture>();

		if (jsonObject.contains("texture"))
		{
			if (!jsonObject["texture"].is_array())
				throw error("JSON format error.");
			for (auto &singleTexture : jsonObject["texture"])
			{
				if (!singleTexture.is_object())
					throw error("JSON format error.");
				if (!(singleTexture.contains("name") && singleTexture.contains("file") && singleTexture.contains("type")))
					throw error("JSON format error.");
				if (!(singleTexture["file"].is_string() && singleTexture["type"].is_string()))
					throw error("Value type error.");
				textureList->insert_or_assign(singleTexture["name"].get<string>(),
				texture(singleTexture["file"].get<string>(), singleTexture["type"].get<string>()));
			}
		}
	}

	// objectArray
	objectArray::objectArray(const char *filename, bool gen/* = true*/)
	{
		// This function encounters problems, probably because of a relative path
		// Judge file type
		string fn = filename;
		string postfix = fn.substr(fn.find_first_of('.') + 1);
		if (postfix.compare("json") == 0)
		{
			ifstream file(filename);
			json jsonFile = json::parse(file);
			file.close();
			genArray(jsonFile, gen);
		}
	}
	objectArray::objectArray(const string &filename, bool gen/* = true*/)
	{
		// Judge file type
		string fn = filename;
		string postfix = fn.substr(fn.find_first_of('.') + 1);
		if (postfix.compare("json") == 0)
		{
			ifstream file(filename);
			json jsonFile = json::parse(file);
			file.close();
			genArray(jsonFile, gen);
		}
	}
	objectArray::objectArray(ifstream &file, bool gen/* = true*/)
	{
		json jsonFile = json::parse(file);
		genArray(jsonFile, gen);
	}
	objectArray::objectArray(const json &jsonObject, bool gen/* = true*/)
	{
		genArray(jsonObject, gen);
	}
	objectArray::~objectArray()
	{
		for (auto i : defination)
		{
			for (auto j : i.second)
			{
				delete j.vArray;
				delete j.iArray;
				delete j.textureList;
			}
		}
	}

	void objectArray::genArray(const json &jsonObject, bool gen)
	{
		if (!jsonObject.is_array())
			throw error("JSON format error.", "JSON root node is not array.");

		for (GLuint i = 0; i < jsonObject.size(); i++)
		{
			// Basic confirm, json object with "type" and "name"
			// "type" is string and "name" is string
			if (jsonObject[i].contains("type") && jsonObject[i].contains("name") &&
				jsonObject[i]["type"].is_string() && jsonObject[i]["name"].is_string())
			{
				string type = jsonObject[i]["type"].get<string>();
				string name = jsonObject[i]["name"].get<string>();
				// This is a defination to a object
				// Defines the vertices of object
				// Contains value of VBO and EBO
				if (type.compare("defination") == 0)
				{
					// Create a singleObject for further resolve
					genDefination(jsonObject[i], name);
				}
				// This is a usage to a object
				// Contains coordinates to transform object to
				else if (type.compare("usage") == 0)
				{
					genUsage(jsonObject[i], name);
				}
				else
					throw error("JSON format error.", "Encountered unfamiliar object.");
			}
			else
				throw error("JSON format error.", "Encountered undefined object.");
		}
		if (gen)
		{
			for (auto &i : defination)
			{
				for (auto j : i.second)
				{
					j.genObjectBuffer();
				}
			}
		}
	}

	void objectArray::genDefination(const json &jsonObject, const string &name)
	{
		if (jsonObject.contains("model") && jsonObject["model"].is_object())
		{
			if (jsonObject["model"].contains("name") && jsonObject["model"]["name"].is_string())
			{
				scene raw(jsonObject["model"]["name"].get<string>());
				function<singleObject&& (const plainModel&)> func = &genObject;
				defination[name] = raw.map(func);
			}
			if (jsonObject.contains("shader") && jsonObject["shader"].is_object())
			{
				for (auto &obj : defination[name])
				{
					obj.sProgram = new shaderProgram(jsonObject["shader"]);
				}
			}
		}
		else
		{
			defination[name] = vector<singleObject>({singleObject(jsonObject)});
			defination[name][0].genObjectBuffer();
		}
	}

	singleObject&& objectArray::genObject(const plainModel &model)
	{
		singleObject *obj = new singleObject();

		auto vRaw = model.rawVertex();
		auto vSize = model.rawVertexSize();
		auto iRaw = model.rawIndice();
		auto iSize = model.rawIndiceSize();
		obj->vArray = new vertexArray(vector<GLfloat>(vRaw, vRaw + vSize));
		obj->iArray = new indiceArray(vector<GLuint>(iRaw, iRaw + iSize));
		obj->sProgram = NULL;
		obj->textureList = new map<string, texture>(model.textures);
		obj->genObjectBuffer();

		return move(*obj);
	}

	void objectArray::genUsage(const json &jsonObject, const string &name)
	{
		// Usage generation
		const string attrName = "attribute";
		// Attributes array
		if (jsonObject[attrName].is_array())
		{
			if (usage.find(name) == usage.end())
				usage.insert(make_pair(name, vector<objectUsage>()));
			usage[name].reserve(jsonObject[attrName].size() + usage[name].size());
			for (GLuint i = 0; i < jsonObject[attrName].size(); i++)
			{
				auto &attrObject = jsonObject[attrName][i];
				if (attrObject.is_object())
				{
					usage[name].emplace_back(genUsageAttr(attrObject, name, i));
				}
				else
					throw error("JSON format error.");
			}
		}
		else
			throw error("JSON format error.");
	}
	objectArray::objectUsage objectArray::genUsageAttr(const json &jsonObject, const string &name, GLuint id)
	{
		objectUsage ret = objectUsage();
		if (jsonObject.contains("model"))
		{
			if (jsonObject["model"].is_array() && jsonObject["model"].size() == 3)
			{
				vector<GLfloat> temp = jsonObject["model"].get<vector<GLfloat>>();
				ret.model = glm::vec3(temp[0], temp[1], temp[2]);
			}
			else
				throw error("JSON format error.");
		}
		else
			throw error("JSON format error.");
		if (jsonObject.contains("rotate"))
		{
			if (jsonObject["rotate"].is_object() && jsonObject["rotate"].contains("axis") && jsonObject["rotate"].contains("degree"))
			{
				vector<GLfloat> temp = jsonObject["rotate"]["axis"].get<vector<GLfloat>>();
				ret.rotateAxis = glm::vec3(temp[0], temp[1], temp[2]);
				ret.rotateDegree = jsonObject["rotate"]["degree"].get<GLfloat>();
			}
			else
				throw error("JSON format error.");
		}
		else
		{
			ret.rotateAxis = glm::vec3(0.0f);
			ret.rotateDegree = 0.0f;
		}
		if (jsonObject.contains("light"))
		{
			if (jsonObject["light"].is_object())
			{
				if (jsonObject["light"].contains("material") && jsonObject["light"]["material"].is_object())
				{
					if (jsonObject["light"]["material"]["ambient"].is_array())
					{
						vector<GLfloat> temp = jsonObject["light"]["material"]["ambient"].get<vector<GLfloat>>();
						ret.material.ambient = glm::vec3(temp[0], temp[1], temp[2]);
					}
					if (jsonObject["light"]["material"]["ambient"].is_array())
					{
						vector<GLfloat> temp = jsonObject["light"]["material"]["diffuse"].get<vector<GLfloat>>();
						ret.material.diffuse = glm::vec3(temp[0], temp[1], temp[2]);
					}
					if (jsonObject["light"]["material"]["ambient"].is_array())
					{
						vector<GLfloat> temp = jsonObject["light"]["material"]["specular"].get<vector<GLfloat>>();
						ret.material.specular = glm::vec3(temp[0], temp[1], temp[2]);
					}
					ret.material.shininess = jsonObject["light"]["material"]["shininess"].get<GLfloat>();
				}
				else
				{
					ret.material.ambient = glm::vec3(1.0f);
					ret.material.diffuse = glm::vec3(1.0f);
					ret.material.specular = glm::vec3(1.0f);
					ret.material.shininess = 32.0f;
				}
				if (jsonObject["light"].contains("source") && jsonObject["light"]["source"].is_string() &&
					jsonObject["light"].contains("color") && jsonObject["light"]["color"].is_array() &&
					jsonObject["light"].contains("attenuation") && jsonObject["light"]["attenuation"].is_array() &&
					jsonObject["light"].contains("strength") && jsonObject["light"]["strength"].is_array())
				{
					lightUsage usage(name, id, ret.model);

					vector<GLfloat> temp = jsonObject["light"]["color"].get<vector<GLfloat>>();
					usage.color = glm::vec3(temp[0], temp[1], temp[2]);
					ret.color = new glm::vec3(temp[0], temp[1], temp[2]);

					temp = jsonObject["light"]["strength"].get<vector<GLfloat>>();
					usage.strength = glm::vec3(temp[0], temp[1], temp[2]);

					temp = jsonObject["light"]["attenuation"].get<vector<GLfloat>>();
					usage.attenuation = glm::vec3(temp[0], temp[1], temp[2]);

					if (jsonObject["light"].contains("direction") && jsonObject["light"]["direction"].is_array())
					{
						temp = jsonObject["light"]["direction"].get<vector<GLfloat>>();
						usage.direction = glm::vec3(temp[0], temp[1], temp[2]);
					}
					if (jsonObject["light"].contains("cutoff") && jsonObject["light"]["cutoff"].is_number())
					{
						usage.cutoff = glm::cos(glm::radians(jsonObject["light"]["cutoff"].get<GLfloat>()));
					}
					if (jsonObject["light"].contains("outerCutoff") && jsonObject["light"]["outerCutoff"].is_number())
					{
						usage.outerCutoff = glm::cos(glm::radians(jsonObject["light"]["outerCutoff"].get<GLfloat>()));
					}
					lightSource.emplace(make_pair(jsonObject["light"]["source"].get<string>(), usage));
				}
			}
			else
				throw error("JSON format error.");
		}
		else
		{
			ret.material.ambient = glm::vec3(1.0f);
			ret.material.diffuse = glm::vec3(1.0f);
			ret.material.specular = glm::vec3(1.0f);
			ret.material.shininess = 32.0f;
		}

		return ret;
	}

	void objectArray::draw(const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &viewPos, const glm::vec3 &viewFacing, void *globalInfo)
	{
		GLenum textureUnit = GL_TEXTURE0;
		for (auto def: defination)
		{
			for (auto single: def.second)
			{
				auto &sProgram = single.getShaderProgram();
				sProgram.useProgram();
				sProgram["viewPos"] = viewPos;
				sProgram["viewFacing"] = viewFacing;
				sProgram["view"] = view;
				sProgram["projection"] = projection;
				for (auto &singleTexture : single.getTextureList())
				{
					if (textureUnit > GL_TEXTURE16)
						throw error("Too many texture unit.");
					glActiveTexture(textureUnit);
					singleTexture.second.useTexture();
					const uniformSetter &setter = sProgram[singleTexture.first];
					setter = {(int)(textureUnit - GL_TEXTURE0)};
					textureUnit++;
				}
				if (usage.count(def.first) == 0)
					continue;
				for (auto light : lightSource)
				{
					glm::vec4 pos = glm::vec4(light.second.pos, 1.0f);
					if (usage[light.second.name][light.second.id].callback != NULL)
						pos = usage[light.second.name][light.second.id].callback(globalInfo) * pos;
					sProgram[light.first + ".pos"] = pos;
					sProgram[light.first + ".color"] = light.second.color;
					sProgram[light.first + ".strength"] = light.second.strength;
					sProgram[light.first + ".attenuation"] = light.second.attenuation;
					sProgram[light.first + ".direction"] = light.second.direction;
					const auto &tempA = sProgram[light.first + ".cutoff"];
					tempA = {light.second.cutoff};
					const auto &tempB = sProgram[light.first + ".outerCutoff"];
					tempB = {light.second.outerCutoff};
				}
				for (auto singleUsage: usage[def.first])
				{
					glm::mat4 model(1.0f);
					model = glm::translate(model, singleUsage.model);
					model = glm::rotate(model, glm::degrees(singleUsage.rotateDegree), singleUsage.rotateAxis);
					if (singleUsage.callback != NULL)
						model *= singleUsage.callback(globalInfo);
					sProgram["model"] = model;
					glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model)));
					sProgram["normalMat"] = normalMat;
					if (singleUsage.color == NULL)
					{
						sProgram["material.ambient"] = singleUsage.material.ambient;
						sProgram["material.diffuse"] = singleUsage.material.diffuse;
						sProgram["material.specular"] = singleUsage.material.specular;
						const auto &temp = sProgram["material.shininess"];
						temp = {singleUsage.material.shininess};
					}
					else
					{
						sProgram["color"] = *singleUsage.color;
					}
					single.draw();
				}
			}
		}
	}

	auto& objectArray::operator[](const string &str)
	{
		return defination[str];
	}
	const auto& objectArray::operator[](const string &str) const
	{
		return defination.at(str);
	}

	map<string, vector<singleObject>>& objectArray::getDefination()
	{
		return defination;
	}
	const map<string, vector<singleObject>>& objectArray::getDefination() const
	{
		return defination;
	}
	map<string, vector<objectArray::objectUsage>>& objectArray::getUsage()
	{
		return usage;
	}
	const map<string, vector<objectArray::objectUsage>>& objectArray::getUsage() const
	{
		return usage;
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
}
