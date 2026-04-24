#version 460 core

in vec2 texCoords;

layout(binding = 0) uniform sampler2D color_tex;

out vec4 fragColor;

#define MAX_FRAGMENTS 128
#define PPLL_NULL_NODE 0xffffffff
struct ListNode
{
    //vec4 color;
	uint color;
    float depth;
    uint next;
};

layout(binding = 0, r32ui) uniform uimage2D ppll_head_tex; // link list heads

layout(binding = 0, std430) buffer PPLL {
    ListNode ppll_nodes[];
};

void insertionSort(inout ListNode sortedFragments[MAX_FRAGMENTS], uint size)
{
	for (uint k = 1; k < size; k++)
	{
		uint j = k;
		ListNode t = sortedFragments[k];
			
		while (sortedFragments[j - 1].depth < t.depth)
		{
			sortedFragments[j] = sortedFragments[j - 1];
			j--;
			if (j <= 0) { break; }
		}

		if (j != k) { sortedFragments[j] = t; }
	}
}

void main() {
    uint index = imageLoad(ppll_head_tex, ivec2(gl_FragCoord.xy)).x;
	/*if(index == PPLL_NULL_NODE) {
		fragColor = vec4(0.0, 1.0, 0.0, 1.0);
	} else
	{
		fragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	return;*/

    if(index == PPLL_NULL_NODE) //discard;
	{
		//fragColor = texture(color_tex, texCoords);
		fragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}


    ListNode frag_nodes[MAX_FRAGMENTS];

    int counter = 0;
    while(index != PPLL_NULL_NODE && counter < MAX_FRAGMENTS)
	{
        frag_nodes[counter] = ppll_nodes[index];
		counter++;
		index = ppll_nodes[index].next;
	}

	// insert sort, descending; uint cause memory
    for(int k = 1; k < counter; k++)
	{
		ListNode t = frag_nodes[k];

		int j = k - 1;

		for(; j >= 0 && frag_nodes[j].depth > t.depth; --j){
			frag_nodes[j + 1] = frag_nodes[j];
		}

		frag_nodes[j + 1] = t;
	}

	//insertionSort(frag_nodes, counter);

	// blend from furthest to closest
	/*vec3 resolved_color = texture(color_tex, texCoords).rgb; // vec3(0.0);
	for(uint k = 0; k < counter; k++)
	{
		vec4 c = frag_nodes[k].color;
		resolved_color = (1.0 - c.a) * resolved_color + c.a * c.rgb;

		//resolved_color = mix(resolved_color, c, c.a);
	}*/

	vec4 unpack_color = unpackUnorm4x8(frag_nodes[0].color);
	vec3 resolved_color = unpack_color.rgb * unpack_color.a; // vec3(0.0);
	float alpha = 1.0 - unpack_color.a;
	for(uint k = 1; k < counter; k++)
	{
		vec4 c = unpackUnorm4x8(frag_nodes[k].color);
		resolved_color = resolved_color + alpha * c.a * c.rgb;
		alpha *= 1.0 - c.a;
		
	}

    //fragColor = ppll_nodes[index].color;

	fragColor = vec4(resolved_color, alpha);

	//fragColor = texture(color_tex, texCoords);
}