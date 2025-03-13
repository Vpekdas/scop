#shader vertex
#version 330 core

layout(location = 0) in vec3 position;

// model * view * projection
uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;

void main() {
    gl_Position = modelMatrix * projectionMatrix * vec4(position, 1.0);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

void main() {
    color = vec4(1.0, 0.0, 0.0, 1.0);
}
