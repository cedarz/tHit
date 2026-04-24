#version 460 core

out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D mraMap;
uniform sampler2D depthMap;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform mat4 lightVP;

uniform vec3 camPos;

const float PI = 3.14159265359;


float lambertian(vec3 surface_normal, vec3 light_normal) {
    return max(dot(surface_normal, light_normal), 0.0f);
}

vec2 poisson_disk[64] = {
    vec2 ( 0.170019  , -0.040254 ) , vec2 ( -0.299417 , 0.791925  ) , vec2 ( 0.645680  , 0.493210  ) ,
    vec2 ( -0.651784 , 0.717887  ) , vec2 ( 0.421003  , 0.027070  ) , vec2 ( -0.817194 , -0.271096 ) ,
    vec2 ( -0.705374 , -0.668203 ) , vec2 ( 0.977050  , -0.108615 ) , vec2 ( 0.063326  , 0.142369  ) ,
    vec2 ( 0.203528  , 0.214331  ) , vec2 ( -0.667531 , 0.326090  ) , vec2 ( -0.098422 , -0.295755 ) ,
    vec2 ( -0.885922 , 0.215369  ) , vec2 ( 0.566637  , 0.605213  ) , vec2 ( 0.039766  , -0.396100 ) ,
    vec2 ( 0.751946  , 0.453352  ) , vec2 ( 0.078707  , -0.715323 ) , vec2 ( -0.075838 , -0.529344 ) ,
    vec2 ( 0.724479  , -0.580798 ) , vec2 ( 0.222999  , -0.215125 ) , vec2 ( -0.467574 , -0.405438 ) ,
    vec2 ( -0.248268 , -0.814753 ) , vec2 ( 0.354411  , -0.887570 ) , vec2 ( 0.175817  , 0.382366  ) ,
    vec2 ( 0.487472  , -0.063082 ) , vec2 ( -0.084078 , 0.898312  ) , vec2 ( 0.488876  , -0.783441 ) ,
    vec2 ( 0.470016  , 0.217933  ) , vec2 ( -0.696890 , -0.549791 ) , vec2 ( -0.149693 , 0.605762  ) ,
    vec2 ( 0.034211  , 0.979980  ) , vec2 ( 0.503098  , -0.308878 ) , vec2 ( -0.016205 , -0.872921 ) ,
    vec2 ( 0.385784  , -0.393902 ) , vec2 ( -0.146886 , -0.859249 ) , vec2 ( 0.643361  , 0.164098  ) ,
    vec2 ( 0.634388  , -0.049471 ) , vec2 ( -0.688894 , 0.007843  ) , vec2 ( 0.464034  , -0.188818 ) ,
    vec2 ( -0.440840 , 0.137486  ) , vec2 ( 0.364483  , 0.511704  ) , vec2 ( 0.034028  , 0.325968  ) ,
    vec2 ( 0.099094  , -0.308023 ) , vec2 ( 0.693960  , -0.366253 ) , vec2 ( 0.678884  , -0.204688 ) ,
    vec2 ( 0.001801  , 0.780328  ) , vec2 ( 0.145177  , -0.898984 ) , vec2 ( 0.062655  , -0.611866 ) ,
    vec2 ( 0.315226  , -0.604297 ) , vec2 ( -0.780145 , 0.486251  ) , vec2 ( -0.371868 , 0.882138  ) ,
    vec2 ( 0.200476  , 0.494430  ) , vec2 ( -0.494552 , -0.711051 ) , vec2 ( 0.612476  , 0.705252  ) ,
    vec2 ( -0.578845 , -0.768792 ) , vec2 ( -0.772454 , -0.090976 ) , vec2 ( 0.504440  , 0.372295  ) ,
    vec2 ( 0.155736  , 0.065157  ) , vec2 ( 0.391522  , 0.849605  ) , vec2 ( -0.620106 , -0.328104 ) ,
    vec2 ( 0.789239  , -0.419965 ) , vec2 ( -0.545396 , 0.538133  ) , vec2 ( -0.178564 , -0.596057 ) ,
    vec2 ( -0.613392 , 0.617481  )
};

vec4 tex2Dproj(sampler2D image, vec4 position, vec2 displacement) {
    vec4 texel = vec4(1.0f);
    vec3 projected_position = position.xyz / position.w;
    texel = texture(image, projected_position.st + displacement);
    return texel;
}

float filter_shadows(sampler2D shadow_map, // the non-linearized shadow map.
                     vec4 light_space_frag, // fragment in light coordiante.
                     float kernel_width, // size of the uniform PCF kernels.
                     float shadow_map_bias) { // bias to remove shadow acne.
    float visibility = 1.0f;

    vec2 shadow_map_size = textureSize(shadow_map, 0) / 2;

    float light_depth = light_space_frag.z / light_space_frag.w;

    float kernel_range = (kernel_width - 1.0f) / 2.0f;
    float kernel_weight = kernel_width * kernel_width;

    int s = 0; // For sampling from a Poisson Disk :-)

    //float shadow_depth = tex2Dproj(shadow_map, light_space_frag, vec2(0.0)).r;
    //if (light_depth < shadow_depth)
    //   return 1.0;
    //return 0.0;

    for (float y = -kernel_range; y <= +kernel_range; y += 1.0f)
    for (float x = -kernel_range; x <= +kernel_range; x += 1.0f) {
        vec2 sample_position = vec2(x,  y);
        //if (pcf_shadows_sampling_type == 1) {
            sample_position += poisson_disk[s];
        //} 
        sample_position /= shadow_map_size;

        float shadow_depth = tex2Dproj(shadow_map, light_space_frag, sample_position).r;
        if (shadow_depth > light_depth - shadow_map_bias)
            visibility -= 1.0f / kernel_weight;
        ++s; // Fetch the next sample position.
    }

    return 1.0f - visibility;
}

void main()
{
    vec3 N   = normalize(Normal);
    vec3 LightPos = vec3(94.26411, 264.5282, 92.26411);//vec3(5.0, 5.0, 5.0);
    vec3 L = normalize(LightPos - WorldPos);
    float lambert = lambertian(N, L);

    vec4 shadow_space_fragment = lightVP * vec4(WorldPos, 1.0);
    float depth_bias = 0.0005;
    float kernel_width = 4;
    float occlusion = filter_shadows(depthMap, shadow_space_fragment, kernel_width, depth_bias);

    vec3 white = vec3(occlusion);
    FragColor = vec4(white * lambert, 1.0);
}


