#include "Shader.h"
#include <fstream>	// for std::ifstream
#include <sstream>	// for std::ostringstream
#include <iostream>

Shader::Shader()
  : mVertShaderID(0),
	mFragShaderID(0),
	mShaderProgramID(0) {}

Shader::~Shader()
{
}

bool Shader::Load(const std::string& vShaderFileName, const std::string& fShaderFileName)
{
	if (!CompileShader(vShaderFileName, GL_VERTEX_SHADER, mVertShaderID) ||
		!CompileShader(fShaderFileName, GL_FRAGMENT_SHADER, mFragShaderID))
	{
		return false;
	}

	mShaderProgramID = glCreateProgram();
	glAttachShader(mShaderProgramID, mVertShaderID);
	glAttachShader(mShaderProgramID, mFragShaderID);
	glLinkProgram(mShaderProgramID);

	if (!IsValidProgram(mShaderProgramID)) { return false; }

	return true;
}

void Shader::Unload()
{
	glDeleteProgram(mShaderProgramID);
	glDeleteShader(mVertShaderID);
	glDeleteShader(mFragShaderID);
}

void Shader::SetActive()
{
	glUseProgram(mShaderProgramID);
}

bool Shader::SetMatrixUniform(const std::string& name, const float* matrix)
{
	GLuint loc = glGetUniformLocation(mShaderProgramID, name.c_str());
	if (loc == -1)
	{
		std::cerr << "Could not bind uniform variable: " << name << std::endl;
		return false;
	}
	glUniformMatrix4fv(loc, 1, GL_FALSE, matrix);

	return true;
}

bool Shader::SetVector3Uniform(const std::string& name, const float* vector)
{
	GLuint loc = glGetUniformLocation(mShaderProgramID, name.c_str());
	if (loc == -1)
	{
		std::cerr << "Could not bind uniform variable: " << name << std::endl;
		return false;
	}
	glUniform3fv(loc, 1, vector);
	return true;
}

bool Shader::SetFloatUniform(const std::string& name, const float value)
{
	GLuint loc = glGetUniformLocation(mShaderProgramID, name.c_str());
	if (loc == -1)
	{
		std::cerr << "Could not bind uniform variable: " << name << std::endl;
		return false;
	}
	glUniform1f(loc, value);
	return true;
}

bool Shader::SetIntUniform(const std::string& name, const int value)
{
	GLuint loc = glGetUniformLocation(mShaderProgramID, name.c_str());
	if (loc == -1)
	{
		std::cerr << "Could not bind uniform variable: " << name << std::endl;
		return false;
	}
	glUniform1i(loc, value);
	return true;
}

bool Shader::CompileShader(const std::string& fileName, GLenum shaderType, GLuint& outShader)
{
	std::ifstream ifs(fileName);
	if (!ifs)
	{
		std::cout << "Shader file not found : " << fileName << std::endl;
		return false;
	}

	std::ostringstream oss;
	oss << ifs.rdbuf();
	std::string contents = oss.str();
	const char* contentsChar = contents.c_str();

	outShader = glCreateShader(shaderType);
	glShaderSource(outShader, 1, &(contentsChar), nullptr);
	glCompileShader(outShader);

	if (!IsCompiled(outShader))
	{
		std::cout << "Failed to compile shader : " << fileName << std::endl;
		return false;
	}

	return true;
}

bool Shader::IsCompiled(GLuint shader)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE)
	{
		char buffer[512];
		std::fill(std::begin(buffer), std::end(buffer), 0);
		glGetShaderInfoLog(shader, 511, nullptr, buffer);
		std::string message(buffer);
		std::cout << "GLSL Compile failed:\n"
				  << message << std::endl;
		return false;
	}

	return true;
}

bool Shader::IsValidProgram(const GLuint programID)
{
	GLint status;
	glGetProgramiv(programID, GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		char buffer[512];
		std::fill(std::begin(buffer), std::end(buffer), 0);
		glGetProgramInfoLog(programID, 511, nullptr, buffer);
		std::string message(buffer);
		std::cout << "GLSL Link status:\n"
				  << message << std::endl;
		return false;
	}

	return true;
}