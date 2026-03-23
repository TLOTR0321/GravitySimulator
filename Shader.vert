#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 color;  // 直接使用uniform颜色

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    Color = color;  // 总是使用uniform颜色
    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}