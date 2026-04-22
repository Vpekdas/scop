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

uniform sampler2D u_Texture;
uniform float u_ModeFactor;

uniform sampler2D u_DissolveTexture;
uniform float u_DissolveAmount;
uniform float u_BurnSize = 0.15f;
uniform float u_BurnBrightness = 0.7f;

const float SMALL_NUMBER = 0.0001f;

in vec2 v_TexCoord;

// https://agatedragon.blog/2025/10/31/creating-a-dissolve-shader/

void main() {

    // 42 determines the number of unique gray shades before the pattern repeats,
    // while 0.2 controls the range of gray shades, ensuring they stay between 0.4 and 0.6.
    float grayShade = 0.4 + mod(float(gl_PrimitiveID), 42.0) / 42.0 * 0.2;

    vec3 baseGray = vec3(grayShade);

    vec3 textureColor = texture(u_Texture, v_TexCoord).rgb;

    // Mix allows us to make a color more "visible" than the other, so when its set to 1.0f,
    // we will see the texture.
    vec3 finalColor = mix(baseGray, textureColor, u_ModeFactor);

    float value = texture(u_DissolveTexture, v_TexCoord).r;
    value *= 0.999f;
    float isVisible = value - u_DissolveAmount;
    if (isVisible < 0.0f) {
        discard;
    } 
    color = vec4(finalColor, 1.0f);
    float isBurning = smoothstep(abs(u_BurnSize) + SMALL_NUMBER, 0.0f, isVisible) * step(SMALL_NUMBER, u_DissolveAmount);
    vec3 burnColor = vec3(0.0, 1.0, 1.0);
    color.rgb += burnColor * isBurning * u_BurnBrightness;
}