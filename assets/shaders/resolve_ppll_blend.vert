#version 460 core

out gl_PerVertex {
	vec4 gl_Position;
	float gl_PointSize;
  float gl_ClipDistance[];
};

out vec2 texCoords;
// https://github.com/SaschaWillems/Vulkan/blob/master/shaders/glsl/deferred/deferred.vert
void main() 
{
	texCoords = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(texCoords * 2.0f - 1.0f, 0.0f, 1.0f);
}