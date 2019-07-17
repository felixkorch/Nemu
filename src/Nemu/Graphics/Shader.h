#pragma once
#include "Nemu/Graphics/OpenGL.h"
#include "glm/glm.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace nemu::graphics {

	class Shader {
		unsigned handle;

		struct ShaderProgramSource {
			std::string VertexSource;
			std::string FragmentSource;
		};

		void CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
		{
			if (vertexShader.empty() || fragmentShader.empty())
				std::cout << "Vertex or Fragment shader empty." << std::endl;

			unsigned int program = glCreateProgram();

			if (program == 0)
				std::cout << "Program failed to compile." << std::endl;

			unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
			unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

			glAttachShader(program, vs);
			glAttachShader(program, fs);
			glLinkProgram(program);
			glValidateProgram(program);

			glDeleteShader(vs);
			glDeleteShader(fs);

			handle = program;
		}

		unsigned int CompileShader(unsigned int type, const std::string& source)
		{
			unsigned int id = glCreateShader(type);
			const char* src = source.c_str();
			glShaderSource(id, 1, &src, nullptr);
			glCompileShader(id);

			int result;
			glGetShaderiv(id, GL_COMPILE_STATUS, &result);

			if (result == GL_FALSE) {
				int length;
				glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
				char* message = (char*)alloca(length * sizeof(char));
				glGetShaderInfoLog(id, length, &length, message);
				std::cout << message << ", " << type << std::endl;
				glDeleteShader(id);
				return 0;
			}

			return id;
		}

		ShaderProgramSource ParseShader(std::stringstream& str)
		{
			enum class ShaderType {
				NONE = -1, VERTEX = 0, FRAGMENT = 1
			};

			std::string line;
			std::stringstream ss[2];
			ShaderType type = ShaderType::NONE;

			while (std::getline(str, line)) {
				if (line == "")
					continue;
				if (line.find("#shader") != std::string::npos) {

					if (line.find("vertex") != std::string::npos)
						type = ShaderType::VERTEX;

					else if (line.find("fragment") != std::string::npos)
						type = ShaderType::FRAGMENT;
				}
				else {
					if (type == ShaderType::NONE) {
						std::cout << "Invalid shader format. Needs to contain vertex / fragments directives." << std::endl;
						return {};
					}
					ss[(int)type] << line << '\n';
				}
			}

			return { ss[0].str(), ss[1].str() };
		}

		int GetUniformLocation(const std::string& name)
		{
			int location = glGetUniformLocation(handle, name.c_str());
			if (location == -1)
				std::cout << "Uniform doesn't exist." << std::endl;
			return location;
		}

	public:

		Shader()
			: handle(0)
		{}

		Shader(Shader& other) = delete;
		Shader(Shader&& other) = delete;
		Shader& operator=(Shader&& other) = delete;
		Shader& operator=(Shader& other) = delete;

		~Shader()
		{
			glDeleteProgram(handle);
		}

		void Bind() const
		{
			glUseProgram(handle);
		}

		void Unbind() const
		{
			glUseProgram(0);
		}

		void SetUniform1i(const std::string& name, int v0)
		{
			glUniform1i(GetUniformLocation(name), v0);
		}

		void SetUniform1iv(const std::string& name, int count, const int* value)
		{
			glUniform1iv(GetUniformLocation(name), count, value);
		}

		void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
		{
			glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
		}

		void SetUniform4f(const std::string& name, const glm::vec4& vec4)
		{
			glUniform4f(GetUniformLocation(name), vec4.x, vec4.y, vec4.z, vec4.w);
		}

		void SetUniformMat4f(const std::string& name, const glm::mat4& mat)
		{
			glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
		}

		void SetUniform1f(const std::string& name, float v0)
		{
			glUniform1f(GetUniformLocation(name), v0);
		}

		void SetUniform3f(const std::string& name, const glm::vec3& vec3)
		{
			glUniform3f(GetUniformLocation(name), vec3.x, vec3.y, vec3.z);
		}

		void LoadFromFile(const std::string& path)
		{
			std::ifstream stream(path);
			if (!stream.good())
				std::cout << "Shader not found, path given: " << path << std::endl;
			std::stringstream str;
			str << stream.rdbuf();
			ShaderProgramSource source = ParseShader(str);
			CreateShader(source.VertexSource, source.FragmentSource);
		}

		void LoadFromString(const char* program)
		{
			std::stringstream str(program);
			ShaderProgramSource source = ParseShader(str);
			CreateShader(source.VertexSource, source.FragmentSource);
		}
	};

}