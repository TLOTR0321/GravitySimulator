#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class Shader {
public:
    Shader() : ID(0) {}
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    // 禁用拷贝构造和赋值
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // 允许移动语义
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void use();
    void setMat4(const std::string& name, const glm::mat4& mat);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec3(const std::string& name, float x, float y, float z);
    void setBool(const std::string& name, bool value);

    // 预取常用uniform位置
    void prefetchCommonUniforms();

private:
    GLuint ID;
    std::unordered_map<std::string, GLint> uniformLocations;

    // 私有方法
    GLint getUniformLocation(const std::string& name);
    void checkCompileErrors(GLuint shader, std::string type);

    // 清理资源
    void cleanup();
};

#endif