#shader vertex
#version 330 core

layout(location = 0) in vec4 position;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_RotationMatrix;

void main() {
    mat4 model = u_ModelMatrix * u_RotationMatrix;
    mat4 MVP = u_ProjectionMatrix * u_ViewMatrix * model;
    gl_Position = MVP * position;
}

#shader fragment
#version 330 core

uniform vec4 u_Color;

layout(location = 0) out vec4 color;

void main() {
    color = u_Color;
}
