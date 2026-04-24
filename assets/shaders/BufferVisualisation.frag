#version 460 core

layout(binding = 0) uniform sampler2D tex;

in vec2 texCoords;

out vec4 fragColor;

void main() {
    vec4 texel = texture(tex, texCoords);
	float depth = texel.x;
	
	fragColor = vec4(depth, depth, depth, 1.0);
	//fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}