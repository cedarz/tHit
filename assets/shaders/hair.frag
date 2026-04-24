#version 460 core

layout(early_fragment_tests) in;

out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 BiTangent;
in vec3 TangentLocal;

// material parameters
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D mraMap;
uniform sampler2D depthMap;

uniform mat4 projection;
uniform mat4 view;
// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform mat4 lightVP;

uniform vec3 camPos;
uniform vec2 windowSize;
uniform float strandWdith;
uniform float u_NearPlane;
uniform float u_FarPlane;


//#define MAX_FRAGMENTS 400 * 400 * 64
#define PPLL_NULL_NODE 0xffffffff
struct ListNode
{
    //vec4 color;
    uint color;
    float depth;
    uint next;
};

layout(binding = 0, r32ui) uniform uimage2D ppll_head_tex; // link list heads
layout(binding = 0, offset = 0) uniform atomic_uint ppll_count;
layout(binding = 0, std430) buffer PPLL {
    ListNode ppll_nodes[];
};

uint NextNodeIndex()
{
    uint index = atomicCounterIncrement(ppll_count);
    uint MAX_FRAGMENTS = uint(windowSize.x) * uint(windowSize.y) * 64;
    if(index < MAX_FRAGMENTS)
    {
        return index;
    }
    else
    {
        return PPLL_NULL_NODE;
    }
}



const float M_PI = 3.14159265359;
const float M_E = 2.71828182845904523536;
// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anyways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------

vec3 getNormal()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Tangent);
    vec3 B  = -normalize(BiTangent);
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------

// https://github.com/Jhonve/HairStrandsRendering/blob/main/HairStrandsRendering/shaders/strands.frag#L57
// view dir from model-position to camera
vec3 kajiya_kay_Y(vec3 diffuse, vec3 specular, float Shininess, vec3 tangent, vec3 light_dir, vec3 view_dir) {
    vec3 Kcd = vec3(1., 1., 1.) * diffuse;
    vec3 Kcs = vec3(1., 1., 1.) * specular;
    vec3 t = normalize(tangent);
    vec3 e = normalize(view_dir);
    float tcose = clamp(dot(t, e), -1., 1.);
    float tsine = sqrt(clamp(1. - tcose * tcose, 0., 1.));

    float tcosl = dot(t, light_dir);
    float tsinl = sqrt(clamp(1. - tcosl * tcosl, 0., 1.));
    return Kcd * tsinl + Kcs * pow((1. - tcose * tcosl + tsine * tsinl) * 0.5, Shininess);
}

// https://github.com/CaffeineViking/vkhr/blob/master/share/shaders/shading/kajiya-kay.glsl
// view dir from camera to model-position, haven't found why?
vec3 kajiya_kay(vec3 diffuse, vec3 specular, float Shininess, vec3 tangent, vec3 light, vec3 eye) {
    float cosTL = dot(tangent, light);
    float cosTE = dot(tangent, eye);

    float cosTL_squared = cosTL*cosTL;
    float cosTE_squared = cosTE*cosTE;

    float one_minus_cosTL_squared = clamp(1.0f - cosTL_squared, 0.0, 1.0);
    float one_minus_cosTE_squared = clamp(1.0f - cosTE_squared, 0.0, 1.0);

    float sinTL = sqrt(one_minus_cosTL_squared);
    float sinTE = sqrt(one_minus_cosTE_squared);

    vec3 diffuse_colors  = diffuse  * sinTL;
    vec3 specular_colors = specular * pow(max(cosTL*cosTE + sinTL*sinTE, 0), Shininess);

    return diffuse_colors + specular_colors;
}

float GPAA1(vec3 world_pos, mat4 view_projection_matrix, vec2 resolution, float strand_width)
{
    vec4 clip_pos = view_projection_matrix * vec4(world_pos, 1.0);
    vec3 ndc = clip_pos.xyz / clip_pos.w;
    vec2 screen_pos = ndc.xy;

    screen_pos = (screen_pos + 1.0) * (resolution / 2.0);

    // distance bwtween pixel center and rasteration point
    float d = length(screen_pos - gl_FragCoord.xy);
    return 1.0 - (d / (strand_width / 2.0)); 
}

float GPAA(vec2 screen_fragment,
           vec4 world_line,
           mat4 view_projection,
           vec2 resolution,
           float line_thickness) {
    // Transforms: world -> clip -> ndc -> screen.
    vec4 clip_line = view_projection * world_line;
    vec3 ndc_line = (clip_line.xyz / clip_line.w);
    vec2 screen_line = ndc_line.xy; // really NDC.

    // Transform NDC to screen-space to compare with samples.
    screen_line = (screen_line + 1.0f) * (resolution / 2.0f);

    // Distance is measured in screen-space grids.
    float d = length(screen_line-screen_fragment);

    // Finally the coverage is based on thickness.
    return 1.0f - (d / (line_thickness / 2.0f));
}

float LinearizeDepth(float vDepth)
{
	float z = vDepth * 2.0 - 1.0; 
	return (2.0 * u_NearPlane * u_FarPlane) / (u_FarPlane + u_NearPlane - z * (u_FarPlane - u_NearPlane));
}

vec4 tex2Dproj(sampler2D image, vec4 position, vec2 displacement) {
    vec4 texel = vec4(1.0f);
    vec3 projected_position = position.xyz / position.w;
    texel = texture(image, projected_position.st + displacement);
    return texel;
}

float approximate_deep_shadow(float shadow_depth, float light_depth, float strand_radius, float strand_alpha) {
    float strand_depth = max(light_depth - shadow_depth, 0.0f); // depth of the current strand inside geometry.
    float strand_count = strand_depth * strand_radius; // expected number of hair strands occluding the strand.

    if (strand_depth > 1e-5) strand_count += 1; // assume we have passed some strand if the depth is above some
    // floating point error threshold (e.g from the given shadow map) and add it to the occluding strand count.

    // We also take into account the transparency of the hair strand to determine how much light might scatter.
    return pow(1.0f - strand_alpha, strand_count); // this gives us "stronger" shadows with deeper hair strand.
}

float approximate_deep_shadows(sampler2D shadow_map, // the non-linearized shadow map itself of the hair style.
                               vec4 light_space_strand, // fragment in the shadow maps light coordinate system.
                               float kernel_width, // size of the PCF kernel, common values are 3x3 or 5x5 too.
                               float smoothing, // this jitter/stride parameter which creates smoother shadows.
                               float strand_radius, // the radius of the hair strands to calculate the density.
                               float strand_opacity) { // inv. proportional to amount of light passing through.
    float visibility = 0.0f;

    vec2 shadow_map_size = textureSize(shadow_map, 0);

    float kernel_range = (kernel_width - 1.0f) / 2.0f;
    float sigma_stddev = (kernel_width / 2.0f) / 2.4f;
    float sigma_squared = sigma_stddev * sigma_stddev;

    float light_depth = light_space_strand.z / light_space_strand.w;
    vec2 shadow_map_stride = shadow_map_size / smoothing; // stride.

    float total_weight = 0.0f;

    for (float y = -kernel_range; y <= +kernel_range; y += 1.0f)
    for (float x = -kernel_range; x <= +kernel_range; x += 1.0f) {
        float exponent = -1.0f * (x*x + y*y) / 2.0f*sigma_squared; // Gaussian RBDF.
        float local_weight =  1.0f / (2.0f*M_PI*sigma_squared) * pow(M_E, exponent);

        float shadow_depth = tex2Dproj(shadow_map, light_space_strand, vec2(x, y) / shadow_map_stride).r;
        float shadow = approximate_deep_shadow(shadow_depth, light_depth, strand_radius, strand_opacity);

        visibility   += shadow * local_weight;
        total_weight += local_weight;
    }

    return visibility / total_weight;
}

void main()
{
    vec3 diffuse = vec3(0.6f, 0.35f, 0.0f); // vec3(0.1, 0.1, 0.1);//
    float p = 80.0;
    vec3 T = normalize(Tangent);
    vec3 L = vec3(1, 1, 1);
    L = normalize(L);
    vec3 V = normalize(camPos - WorldPos);
    vec3 LightPos = lightPositions[0];//vec3(94.26411, 264.5282, 92.26411);// vec3(5.0, 5.0, 5.0);
    L = normalize(LightPos - WorldPos);

    vec3 specular = lightColors[0];//vec3(0.430, 0.380, 0.280);

    vec3 shading = kajiya_kay(diffuse, specular, p, T, L, -V);
    shading = kajiya_kay_Y(diffuse, specular, p, T, L, V);

    float coverage = GPAA1(WorldPos, projection * view, windowSize, strandWdith);
    //float coverage = GPAA(gl_FragCoord.xy, vec4(WorldPos, 1.0), projection * view, windowSize, strandWdith);
    //coverage *= 0.3; // Alpha used for transparency.
    if (coverage < 0.001) discard; // Shading not worth it!
    //coverage *= fs_in.thickness * (1.0 / 0.042); // Slowly fades the strand at the tip.

    vec4 light_frag = lightVP * vec4(WorldPos, 1.0);
    float kernel_width = 4;
    float stride_size = 4;

    float occlusion = approximate_deep_shadows(depthMap, light_frag, kernel_width, stride_size, 3, 0.3);

    // https://github.com/AngelMonica126/GraphicAlgorithm/blob/master/012_Real%20Time%20Concurrent%20Linked%20List%20Construction%20on%20the%20GPU/CreateLinkList_FS.glsl
    uint nextIndex = NextNodeIndex();
    if (nextIndex == PPLL_NULL_NODE) discard;

    uint headIndex = imageAtomicExchange(ppll_head_tex, ivec2(gl_FragCoord.xy), nextIndex);
    float currentDepth = LinearizeDepth(gl_FragCoord.z);
    uint pack_shading = packUnorm4x8(vec4(shading * occlusion, coverage));
    ppll_nodes[nextIndex] = ListNode(pack_shading, currentDepth, headIndex);

    discard;
    FragColor = vec4(shading, coverage);
    //FragColor = vec4(coverage,coverage,coverage, 1.0);
}

// hair shading from tressfx
void main1()
{
    vec3 albedo = vec3(0.6f, 0.35f, 0.0f); //pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));

    vec3 ambient = vec3(0.03);
    
    vec3 V = normalize(camPos - WorldPos);

    // diffuse
    vec3 L = vec3(1, 1, 1);
    L = normalize(L);


    vec3 T = normalize(Tangent);
    //T = normalize(TangentLocal);
    //FragColor = vec4(T * 0.5 + 0.5, 1.0);
    //return;


    float cosTL = dot(T, L);
    float sinTL = sqrt(1.0 - cosTL * cosTL);
    vec3 diffuse = vec3(sinTL);

    float rand_value = 1.0;
    float alpha = (rand_value * 10) * M_PI/180; // tiled angle (5-10 dgree)

    // in Kajiya's model: specular component: cos(t, rl) * cos(t, e) + sin(t, rl) * sin(t, e)
    float cosTRL = -cosTL;
    float sinTRL = sinTL;
    float cosTE = (dot(T, V));
    float sinTE = sqrt(1- cosTE * cosTE);

    // primary highlight: reflected direction shift towards root (2 * Alpha)
    float cosTRL_root = cosTRL * cos(2 * alpha) - sinTRL * sin(2 * alpha);
    float sinTRL_root = sqrt(1 - cosTRL_root * cosTRL_root);
    float specular_root = max(0, cosTRL_root * cosTE + sinTRL_root * sinTE);

    // secondary highlight: reflected direction shifted toward tip (3*Alpha)
    float cosTRL_tip = cosTRL*cos(-3*alpha) - sinTRL*sin(-3*alpha);
    float sinTRL_tip = sqrt(1 - cosTRL_tip * cosTRL_tip);
    float specular_tip = max(0, cosTRL_tip * cosTE + sinTRL_tip * sinTE);

    vec3 color = //ambient * albedo +
                 0.3 * diffuse * albedo + 
                 0.1 * pow(specular_root, 2.0) + 
                 0.1 * pow(specular_tip, 2.0) * albedo;


    // HDR tonemapping
    //color = color / (color + vec3(1.0));

    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);

}
