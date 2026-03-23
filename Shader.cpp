#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

Shader::Shader(const char* vertexPath, const char* fragmentPath) : ID(0) {
    // 读取着色器文件
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 编译着色器
    GLuint vertex, fragment;

    // 顶点着色器
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // 片段着色器
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // 着色器程序
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // 删除着色器
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

// 移动构造函数
Shader::Shader(Shader&& other) noexcept
    : ID(other.ID), uniformLocations(std::move(other.uniformLocations)) {
    other.ID = 0;
}

// 移动赋值运算符
Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        cleanup();
        ID = other.ID;
        uniformLocations = std::move(other.uniformLocations);
        other.ID = 0;
    }
    return *this;
}

Shader::~Shader() {
    cleanup();
}

void Shader::cleanup() {
    if (ID != 0) {
        glDeleteProgram(ID);
        ID = 0;
    }
    uniformLocations.clear();
}

void Shader::use() {
    glUseProgram(ID);
}

// 预取常用uniform位置
void Shader::prefetchCommonUniforms() {
    // 预取最常见的uniform位置
    std::vector<std::string> commonUniforms = {
        "projection", "view", "model",
        "color", "lightPos", "lightColor", "viewPos"
    };

    for (const auto& name : commonUniforms) {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if (location != -1) {
            uniformLocations[name] = location;
        }
    }
}

// 获取uniform位置（带缓存）
GLint Shader::getUniformLocation(const std::string& name) {
    auto it = uniformLocations.find(name);
    if (it != uniformLocations.end()) {
        return it->second;
    }

    GLint location = glGetUniformLocation(ID, name.c_str());
    uniformLocations[name] = location;
    return location;
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
    }
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform3fv(location, 1, &value[0]);
    }
}

void Shader::setVec3(const std::string& name, float x, float y, float z) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform3f(location, x, y, z);
    }
}

void Shader::setBool(const std::string& name, bool value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, (int)value);
    }
}

void Shader::checkCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];

    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                << infoLog << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                << infoLog << std::endl;
        }
    }
}