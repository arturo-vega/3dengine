#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>	
#include <fstream>
#include <sstream>
#include <iostream>

/*
Personal Notes

Shader gets glsl files and compiles, links, and builds the shader from these files. 

*/

class Shader {
public:
	/* Program ID*/
	unsigned int ID;

	/* Reads and builds the shader */
	Shader(const char* vertexPath, const char* fragmentPath) {
		/*1 Retrieve vertex/fragment code from file path*/
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;

		//std::cout << vertextPath << std::endl;
		//std::cout << fragmentPath << std::endl;

		/*Check ifstream objects can throw exceptions*/
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try {
			/* open files */
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			/* read file's buffer contents into streams */
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			/* close file handlers */
			vShaderFile.close();
			fShaderFile.close();
			/* conver stream into string */
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure fuck) {
			std::cout << "ERROR SHADER FILE NOT SUCCESSFULLY READ" << std::endl;
		}

		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		/*2 compile shaders*/
		unsigned int vertex, fragment;
		int success;
		char* infoLog = (char*) malloc(sizeof(char)*1024);

		/* Vertex Shader */
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		/* Print compile errors if they occur */
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
			std::cout << "ERROR SHADER VERTEX COMPILATION FAILED\n" << infoLog << std::endl;
		}

		/* Fragment Shader */
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		/* Print compile errors if they occur */
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(fragment, 1024, NULL, infoLog);
			std::cout << "ERROR SHADER VERTEX COMPILATION FAILED\n" << infoLog << std::endl;
		}

		/* Shader Program */
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);
		/* Print linking errors if they occur */
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(ID, 1024, NULL, infoLog);
			std::cout << "ERROR SHADER PROGRAM LINKING FAILED\n" << infoLog << std::endl;
		}

		glDeleteShader(vertex);
		glDeleteShader(fragment);
		free(infoLog);
	}

	/* Use the shader */
	void use() {
		glUseProgram(ID);
	}
	/* Utility uniform constructor functions */
	void setBool(const std::string &name, bool value) const {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	void setInt(const std::string &name, int value) const {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setFloat(const std::string& name, float value) const {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}

private:
	void checkCompileErrors(unsigned int shader, std::string type) {
		int success;
		char* infoLog = (char*)malloc(sizeof(char*) * 1024);
		if (type != "PROGRAM") {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR SHADER COMPILATION ERROR of type " << type << "\n" << infoLog << std::endl;
			}
		}
		else {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR PROGRAM LINKING ERROR of type " << type << "\n" << infoLog << std::endl;
			}
		}

		free(infoLog);
	}
};


#endif