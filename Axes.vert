#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 2) in vec3 aColor;  // 注意：位置2，因为位置1是法线

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 Color;

void main() {
    Color = aColor;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}