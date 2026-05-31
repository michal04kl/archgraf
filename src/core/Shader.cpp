#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    std::string vertexCode, fragmentCode;
    std::ifstream vShaderFile, fShaderFile;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vss, fss;
        vss << vShaderFile.rdbuf();
        fss << fShaderFile.rdbuf();
        vertexCode   = vss.str();
        fragmentCode = fss.str();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "[Shader] Blad odczytu: " << e.what() << "\n";
        std::cerr << "  vertex:   " << vertexPath   << "\n";
        std::cerr << "  fragment: " << fragmentPath << "\n";
    }

    const char* vCode = vertexCode.c_str();
    const char* fCode = fragmentCode.c_str();

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vCode, nullptr);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fCode, nullptr);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use() const { glUseProgram(ID); }

void Shader::setBool (const std::string& n, bool v)         const { glUniform1i (glGetUniformLocation(ID,n.c_str()),(int)v); }
void Shader::setInt  (const std::string& n, int v)          const { glUniform1i (glGetUniformLocation(ID,n.c_str()),v); }
void Shader::setFloat(const std::string& n, float v)        const { glUniform1f (glGetUniformLocation(ID,n.c_str()),v); }
void Shader::setVec2 (const std::string& n, const glm::vec2& v) const { glUniform2fv(glGetUniformLocation(ID,n.c_str()),1,glm::value_ptr(v)); }
void Shader::setVec3 (const std::string& n, const glm::vec3& v) const { glUniform3fv(glGetUniformLocation(ID,n.c_str()),1,glm::value_ptr(v)); }
void Shader::setVec4 (const std::string& n, const glm::vec4& v) const { glUniform4fv(glGetUniformLocation(ID,n.c_str()),1,glm::value_ptr(v)); }
void Shader::setMat4 (const std::string& n, const glm::mat4& m) const { glUniformMatrix4fv(glGetUniformLocation(ID,n.c_str()),1,GL_FALSE,glm::value_ptr(m)); }

void Shader::checkCompileErrors(unsigned int shader, const std::string& type)
{
    int success; char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) { glGetShaderInfoLog(shader,1024,nullptr,infoLog); std::cerr<<"[Shader] "<<type<<":\n"<<infoLog<<"\n"; }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) { glGetProgramInfoLog(shader,1024,nullptr,infoLog); std::cerr<<"[Shader] LINK:\n"<<infoLog<<"\n"; }
    }
}