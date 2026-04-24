#version 460 core

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
//uniform sampler2D metallicMap;
//uniform sampler2D roughnessMap;
//uniform sampler2D aoMap;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 camPos;

const float PI = 3.14159265359;
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
    denom = PI * denom * denom;

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

// hair shading
void main()
{
    vec3 albedo     = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    vec3 normal = getNormal();

    vec3 ambient = vec3(0.03);
    
    vec3 V = normalize(camPos - WorldPos);

    // diffuse
    vec3 L = vec3(1, 1, 1);
    L = normalize(L);
    vec3 diffuse = vec3(clamp(mix(0.25, 1.0, dot(normal, L)), 0.0, 1.0));


    vec3 T = normalize(Tangent);
    T = normalize(TangentLocal);
    FragColor = vec4(T * 0.5 + 0.5, 1.0);
    return;
    float cosTL = dot(T, L);
    float sinTL = sqrt(1.0 - cosTL * cosTL);
    //diffuse = sinTL;

    float rand_value = 1.0;
    float alpha = (rand_value * 10) * PI/180; // tiled angle (5-10 dgree)

    // in Kajiya's model: specular component: cos(t, rl) * cos(t, e) + sin(t, rl)sin(t, e)
    float cosTRL = -cosTL;
    float sinTRL = sinTL;
    float cosTE = (dot(T, V));
    float sinTE = sqrt(1- cosTE*cosTE);

    // primary highlight: reflected direction shift towards root (2 * Alpha)
    float cosTRL_root = cosTRL * cos(2 * alpha) - sinTRL * sin(2 * alpha);
    float sinTRL_root = sqrt(1 - cosTRL_root * cosTRL_root);
    float specular_root = max(0, cosTRL_root * cosTE + sinTRL_root * sinTE);

    // secondary highlight: reflected direction shifted toward tip (3*Alpha)
    float cosTRL_tip = cosTRL*cos(-3*alpha) - sinTRL*sin(-3*alpha);
    float sinTRL_tip = sqrt(1 - cosTRL_tip * cosTRL_tip);
    float specular_tip = max(0, cosTRL_tip * cosTE + sinTRL_tip * sinTE);

    vec3 color = ambient * albedo +
                 + 0.4 * diffuse * albedo
                 //+ 0.3 * pow(specular_root, 4.0);
                 + 0.3 * pow(specular_tip, 4.0) * albedo;


    // HDR tonemapping
    color = color / (color + vec3(1.0));

    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);

}
void main_pbr()
{		
    vec3 albedo     = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    //float metallic  = texture(metallicMap, TexCoords).r;
    //float roughness = texture(roughnessMap, TexCoords).r;
    //float ao        = texture(aoMap, TexCoords).r;

    vec3 mra = texture(mraMap, TexCoords).rgb;
    float metallic = mra.r;
    float roughness = mra.g;
    float ao = mra.b;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 1; ++i) 
    {
        // calculate per-light radiance
        vec3 L = vec3(1, 1, 1);//normalize(lightPositions[i] - WorldPos);
        L = normalize(L);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = vec3(10,10,10);//lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    //color = texture(albedoMap, TexCoords).rgb;
    //color = vec3(0.3, 0.4, 0.5);
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}

/*out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D albedo;

void main()
{    
    FragColor = texture(albedo, TexCoords);
}*/


/*#include ../../common/shaders/uniforms.glsl

in VS_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoords;
  vec3 tangent;
  vec3 dndu;
  vec3 dndv;
} fs_in;

out vec4 fragColor;

uniform int layerType;

void main() {
  vec3 color = vec3(0);

  if(layerType == 0) {
    color = fs_in.position;
  }
  else if(layerType == 1) {
    color = 0.5 * fs_in.normal + 0.5;
  }
  else if(layerType == 2) {
    color = vec3(fs_in.texCoords, 0.0);
  }
  else if(layerType == 3) {
    color = 0.5 * fs_in.tangent + 0.5;
  }
  else if(layerType == 4) {
    color = 0.5 * fs_in.dndu + 0.5;
  }
  else if(layerType == 5) {
    color = 0.5 * fs_in.dndv + 0.5;
  }
  else if(layerType == 6) {
    color = texture(diffuseMap, fs_in.texCoords).xyz + material.kd;
    // gamma correction
    color = pow(color, vec3(1.0 / 2.2));
  }
  else if(layerType == 7) {
    color = texture(specularMap, fs_in.texCoords).xyz + material.ks;
  }
  else if(layerType == 8) {
    color = texture(ambientMap, fs_in.texCoords).xyz + material.ka;
    // gamma correction
    color = pow(color, vec3(1.0 / 2.2));
  }
  else if(layerType == 9) {
    color = texture(emissiveMap, fs_in.texCoords).xyz + material.ke;
    // gamma correction
    color = pow(color, vec3(1.0 / 2.2));
  }
  else if(layerType == 10) {
    color = texture(heightMap, fs_in.texCoords).xyz;
  }
  else if(layerType == 11) {
    color = texture(normalMap, fs_in.texCoords).xyz;
  }
  else if(layerType == 12) {
    color = texture(shininessMap, fs_in.texCoords).xyz;
  }
  else if(layerType == 13) {
    color = texture(displacementMap, fs_in.texCoords).xyz;
  }
  else if(layerType == 14) {
    color = texture(lightMap, fs_in.texCoords).xyz;
  }

  fragColor = vec4(color, 1.0);
}*/