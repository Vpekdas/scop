#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uvCoords;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

out vec2 uv;

void main() {
    mat4 MVP = u_ProjectionMatrix * u_ViewMatrix * u_ModelMatrix;
    gl_Position = MVP * position;
    uv = uvCoords;

}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;
in vec2 uv;

void main() {
    vec3 uvColor = vec3(uv, 0.5);

    color = vec4(uvColor, 1.0);
}