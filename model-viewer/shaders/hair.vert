#version 460 core

/*layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}*/

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBiTangent;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out vec3 Tangent;
out vec3 BiTangent;
out vec3 TangentLocal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;

void main()
{
    TexCoords = aTexCoords;
    WorldPos = vec3(model * vec4(aPos, 1.0));
    Normal = normalMatrix * aNormal;
    Tangent = aTangent;//mat3(model) * aTangent;
    BiTangent = mat3(model) * aBiTangent;
    TangentLocal = aTangent;

    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}


/*layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;
layout (location = 3) in vec3 vTangent;
layout (location = 4) in vec3 vDndu;
layout (location = 5) in vec3 vDndv;

out gl_PerVertex {
  vec4 gl_Position;
};
out VS_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoords;
  vec3 tangent;
  vec3 dndu;
  vec3 dndv;
} vs_out;

uniform mat4 view;
uniform mat4 projection;

void main() {
  gl_Position = projection * view * vec4(vPosition, 1.0);
  vs_out.position = vPosition;
  vs_out.normal = vNormal;
  vs_out.texCoords = vTexCoords;
  vs_out.tangent = vTangent;
  vs_out.dndu = vDndu;
  vs_out.dndv = vDndv;
}*/