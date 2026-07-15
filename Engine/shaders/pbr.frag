#version 450 core
out vec4 FragColor;

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;
    mat3 TBN;
    vec3 CamPos;
} fs_in;

// Material 
struct MaterialData {
    vec3  albedo;
    float metallic;
    float roughness;
    float ao;
    vec3  emissive;
    float emissiveStrength;
};
uniform MaterialData u_Material;
uniform bool u_UseAlbedoMap;
uniform bool u_UseNormalMap;
uniform bool u_UseMetallicRoughnessMap;
uniform bool u_UseAOMap;
uniform bool u_UseEmissiveMap;
uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicRoughnessMap;
uniform sampler2D u_AOMap;
uniform sampler2D u_EmissiveMap;

// Lights 
struct DirLight { vec4 direction; vec4 color; mat4 lightSpaceMatrix; vec4 shadowParams; };
struct PointLight { vec4 posRange; vec4 colorIntensity; vec4 attenuation; };
struct SpotLight  { vec4 posRange; vec4 dirIntensity; vec4 colorOuter; vec4 params; };

layout(std140, binding=1) uniform Lights {
    DirLight   u_DirLights[4];
    PointLight u_PointLights[32];
    SpotLight  u_SpotLights[8];
    vec4       u_Ambient;
    ivec4      u_LightCounts;
};

uniform sampler2D u_ShadowMaps[4];

// Constants 
const float PI = 3.14159265359;

// Cook-Torrance BRDF 
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float ggx2 = GeometrySchlickGGX(max(dot(N, V), 0.0), roughness);
    float ggx1 = GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ─── Shadow sampling (PCF 3x3) ───────────────────────────────────────────────
float ShadowFactor(sampler2D shadowMap, vec4 fragPosLS, float bias) {
    vec3 proj = fragPosLS.xyz / fragPosLS.w;
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0) return 0.0;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float depth = texture(shadowMap, proj.xy + vec2(x,y) * texelSize).r;
            shadow += (proj.z - bias > depth) ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

// ─── PBR radiance for one light ──────────────────────────────────────────────
vec3 CalcPBR(vec3 N, vec3 V, vec3 L, vec3 radiance,
             vec3 albedo, float metallic, float roughness, vec3 F0) {
    vec3 H = normalize(V + L);
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kD = (1.0 - F) * (1.0 - metallic);
    float NdotL = max(dot(N, L), 0.0);
    vec3 num    = NDF * G * F;
    float denom = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
    vec3 specular = num / denom;

    return (kD * albedo / PI + specular) * radiance * NdotL;
}

void main() {
    // ── Sample material ────────────────────────────────────────────────────
    vec3 albedo = u_Material.albedo;
    if (u_UseAlbedoMap) albedo = pow(texture(u_AlbedoMap, fs_in.TexCoords).rgb, vec3(2.2));

    float metallic  = u_Material.metallic;
    float roughness = u_Material.roughness;
    if (u_UseMetallicRoughnessMap) {
        vec2 mr = texture(u_MetallicRoughnessMap, fs_in.TexCoords).bg;
        metallic  = mr.x;
        roughness = mr.y;
    }

    float ao = u_Material.ao;
    if (u_UseAOMap) ao = texture(u_AOMap, fs_in.TexCoords).r;

    vec3 emissive = u_Material.emissive * u_Material.emissiveStrength;
    if (u_UseEmissiveMap) emissive += texture(u_EmissiveMap, fs_in.TexCoords).rgb;

    // ── Normal mapping ────────────────────────────────────────────────────
    vec3 N = normalize(fs_in.Normal);
    if (u_UseNormalMap) {
        N = texture(u_NormalMap, fs_in.TexCoords).rgb * 2.0 - 1.0;
        N = normalize(fs_in.TBN * N);
    }

    vec3 V = normalize(fs_in.CamPos - fs_in.WorldPos);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo = vec3(0.0);

    // ── Directional lights ────────────────────────────────────────────────
    for (int i = 0; i < u_LightCounts.x; i++) {
        vec3 L = normalize(-u_DirLights[i].direction.xyz);
        vec3 radiance = u_DirLights[i].color.rgb * u_DirLights[i].direction.w;
        vec3 contrib = CalcPBR(N, V, L, radiance, albedo, metallic, roughness, F0);

        float shadow = 0.0;
        if (u_DirLights[i].color.w > 0.5) {
            vec4 fragLS = u_DirLights[i].lightSpaceMatrix * vec4(fs_in.WorldPos, 1.0);
            shadow = ShadowFactor(u_ShadowMaps[i], fragLS, u_DirLights[i].shadowParams.x);
        }
        Lo += contrib * (1.0 - shadow);
    }

    // ── Point lights ──────────────────────────────────────────────────────
    for (int i = 0; i < u_LightCounts.y; i++) {
        vec3 lightPos = u_PointLights[i].posRange.xyz;
        float range   = u_PointLights[i].posRange.w;
        vec3 L = normalize(lightPos - fs_in.WorldPos);
        float dist = length(lightPos - fs_in.WorldPos);
        if (dist > range) continue;

        float c = u_PointLights[i].attenuation.x;
        float l = u_PointLights[i].attenuation.y;
        float q = u_PointLights[i].attenuation.z;
        float atten = 1.0 / (c + l * dist + q * dist * dist);
        float intensity = u_PointLights[i].colorIntensity.w;
        vec3 radiance = u_PointLights[i].colorIntensity.rgb * intensity * atten;
        Lo += CalcPBR(N, V, L, radiance, albedo, metallic, roughness, F0);
    }

    // ── Spot lights ───────────────────────────────────────────────────────
    for (int i = 0; i < u_LightCounts.z; i++) {
        vec3 lightPos  = u_SpotLights[i].posRange.xyz;
        float range    = u_SpotLights[i].posRange.w;
        vec3 L = normalize(lightPos - fs_in.WorldPos);
        float dist = length(lightPos - fs_in.WorldPos);
        if (dist > range) continue;

        vec3 spotDir = normalize(-u_SpotLights[i].dirIntensity.xyz);
        float theta  = dot(L, spotDir);
        float outer  = u_SpotLights[i].colorOuter.w;
        float inner  = u_SpotLights[i].params.x;
        float epsilon = inner - outer;
        float spot = clamp((theta - outer) / epsilon, 0.0, 1.0);

        float intensity = u_SpotLights[i].dirIntensity.w;
        vec3 radiance = u_SpotLights[i].colorOuter.rgb * intensity * spot;
        Lo += CalcPBR(N, V, L, radiance, albedo, metallic, roughness, F0);
    }

    // ── Ambient ───────────────────────────────────────────────────────────
    vec3 ambient = u_Ambient.xyz * u_Ambient.w * albedo * ao;
    vec3 color   = ambient + Lo + emissive;

    // ── Tone mapping (ACES) + gamma ───────────────────────────────────────
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
