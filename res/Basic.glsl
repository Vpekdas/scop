#shader vertex
#version 330 core

layout(location = 0) in vec4 position;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

void main() {
    mat4 MVP = u_ProjectionMatrix * u_ViewMatrix * u_ModelMatrix;
    gl_Position = MVP * position;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

void main() {
    float r = float((gl_PrimitiveID % 3) == 0);
    float g = float((gl_PrimitiveID % 3) == 1);
    float b = float((gl_PrimitiveID % 3) == 2);
    color = vec4(r, g, b, 1.0);
}