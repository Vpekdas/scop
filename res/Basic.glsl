#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

out vec2 v_TexCoord;

void main() {
    mat4 MVP = u_ProjectionMatrix * u_ViewMatrix * u_ModelMatrix;
    gl_Position = MVP * position;
    v_TexCoord = texCoord;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
uniform sampler2D u_Texture;

void main() {
    // float r = float((gl_PrimitiveID % 3) == 0);
    // float g = float((gl_PrimitiveID % 3) == 1);
    // float b = float((gl_PrimitiveID % 3) == 2);
    // color = vec4(r, g, b, 1.0);

    color = texture(u_Texture, v_TexCoord);
}