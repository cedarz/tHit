#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;


uniform mat4 projection;
uniform mat4 view;
//uniform mat4 model;

uniform vec3 Translation;

void main()
{
    vec3 WorldPos = normalize(aPos) * vec3(10.0) + Translation;

    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}


