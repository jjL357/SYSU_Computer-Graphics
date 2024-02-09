#pragma once

#include <GL\glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

//using namespace std;

#define INVALID_UNIFORM_LOCATION -1

class Shader
{
public:
	GLuint program;
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath);
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath,
		const GLchar* varyings[], int count);
	void use();

	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setMat4(const std::string &name, glm::mat4 value)const;
	void setVec3(const std::string &name, glm::vec3 value)const;
	void setVec3(const std::string &name, float x, float y, float z)const;
};