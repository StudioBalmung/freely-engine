#include "Freely/Renderer/Renderer3D.h"
#include "Freely/Renderer/Shader.h"
#include "Freely/Renderer/Framebuffer.h"
#include "Freely/Renderer/UniformBuffer.h"
#include "Freely/Renderer/VertexArray.h"
#include "Freely/Renderer/Buffer.h"
#include "Freely/Renderer/Texture.h"
#include "Freely/Scene/Mesh.h"
#include "Freely/Scene/Material.h"
#include "Freely/Scene/Camera.h"
#include "Freely/Core/Logger.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <array>

namespace Freely {

// ─── Internal state ──────────────────────────────────────────────────────────
namespace R3D {

// UBO binding points
static constexpr uint32_t kCameraBinding = 0;
static constexpr uint32_t kLightsBinding = 1;

// Shadow map resolution
static constexpr uint32_t kShadowSize = 2048;
// Max directional light shadows
static constexpr uint32_t kMaxDirShadows = 4;
// Max point lights in UBO
static constexpr uint32_t kMaxPointLights = 32;
// Max spot lights in UBO
static constexpr uint32_t kMaxSpotLights = 8;

// ─── Camera UBO layout (binding=0, 192 bytes) ────────────────────────────────
struct CameraData {
    glm::mat4 View;
    glm::mat4 Projection;
    glm::mat4 ViewProjection;
    glm::vec4 Position; // w unused
};

// ─── Lights UBO layout (binding=1) ──────────────────────────────────────────
struct GPUDirectionalLight {
    glm::vec4 Direction;    // w = intensity
    glm::vec4 Color;        // w = castShadow (1/0)
    glm::mat4 LightSpaceMatrix;
    glm::vec4 ShadowParams; // x = bias
};
struct GPUPointLight {
    glm::vec4 Position;     // w = range
    glm::vec4 Color;        // w = intensity
    glm::vec4 Attenuation;  // x=constant, y=linear, z=quadratic
};
struct GPUSpotLight {
    glm::vec4 Position;     // w = range
    glm::vec4 Direction;    // w = intensity
    glm::vec4 Color;        // w = outer cutoff cos
    glm::vec4 Params;       // x = inner cutoff cos
};
struct LightsData {
    GPUDirectionalLight DirLights[kMaxDirShadows];
    GPUPointLight       PointLights[kMaxPointLights];
    GPUSpotLight        SpotLights[kMaxSpotLights];
    glm::vec4           Ambient;          // xyz=color, w=strength
    glm::ivec4          LightCounts;      // x=dir, y=point, z=spot
};

struct State {
    std::shared_ptr<Shader> PBRShader;
    std::shared_ptr<Shader> ShadowShader;
    std::shared_ptr<Shader> SkyboxShader;
    std::shared_ptr<Shader> GridShader;

    std::shared_ptr<UniformBuffer> CameraUBO;
    std::shared_ptr<UniformBuffer> LightsUBO;

    // Per-light shadow FBOs and depth textures
    struct ShadowMap {
        uint32_t FBO = 0;
        uint32_t Depth = 0;
    };
    std::array<ShadowMap, kMaxDirShadows> ShadowMaps;

    // Skybox VAO/VBO
    std::shared_ptr<VertexArray> SkyboxVAO;

    // Grid VAO
    std::shared_ptr<VertexArray> GridVAO;

    // Draw queues
    std::vector<DrawCall3D> OpaqueQueue;
    std::vector<DrawCall3D> TransparentQueue;

    // Per-frame scene data
    CameraData  Camera;
    LightsData  Lights;
    glm::mat4   LightSpaceMatrices[kMaxDirShadows];

    uint32_t ViewportW = 1280;
    uint32_t ViewportH = 720;

    RenderStats3D Stats;
    bool Initialized = false;
};

static State s_State;

// ─── Embedded GLSL shaders ───────────────────────────────────────────────────

static const char* kPBR_Vert = R"GLSL(
#version 450 core
layout(location=0) in vec3 a_Position;
layout(location=1) in vec3 a_Normal;
layout(location=2) in vec2 a_TexCoords;
layout(location=3) in vec3 a_Tangent;

layout(std140, binding=0) uniform Camera {
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProjection;
    vec4 u_CamPos;
};

uniform mat4 u_Model;
uniform mat4 u_NormalMatrix;

out VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;
    mat3 TBN;
    vec3 CamPos;
} vs_out;

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    vs_out.WorldPos  = worldPos.xyz;
    vs_out.TexCoords = a_TexCoords;
    vs_out.CamPos    = u_CamPos.xyz;

    vec3 N = normalize(mat3(u_NormalMatrix) * a_Normal);
    vec3 T = normalize(mat3(u_NormalMatrix) * a_Tangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    vs_out.TBN    = mat3(T, B, N);
    vs_out.Normal = N;

    gl_Position = u_ViewProjection * worldPos;
}
)GLSL";

static const char* kPBR_Frag = R"GLSL(
#version 450 core
out vec4 FragColor;

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;
    mat3 TBN;
    vec3 CamPos;
} fs_in;

// ─── Material ────────────────────────────────────────────────────────────────
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

// ─── Lights ──────────────────────────────────────────────────────────────────
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

// ─── Constants ───────────────────────────────────────────────────────────────
const float PI = 3.14159265359;

// ─── Cook-Torrance BRDF ──────────────────────────────────────────────────────
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

// ─── Shadow sampling (PCF 3×3) ───────────────────────────────────────────────
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
)GLSL";

static const char* kShadow_Vert = R"GLSL(
#version 450 core
layout(location=0) in vec3 a_Position;
uniform mat4 u_LightSpaceMatrix;
uniform mat4 u_Model;
void main() {
    gl_Position = u_LightSpaceMatrix * u_Model * vec4(a_Position, 1.0);
}
)GLSL";

static const char* kShadow_Frag = R"GLSL(
#version 450 core
void main() {}
)GLSL";

static const char* kSkybox_Vert = R"GLSL(
#version 450 core
layout(location=0) in vec3 a_Position;
layout(std140, binding=0) uniform Camera {
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProjection;
    vec4 u_CamPos;
};
out vec3 v_TexCoord;
void main() {
    v_TexCoord = a_Position;
    // Remove translation from view matrix
    mat4 rotView = mat4(mat3(u_View));
    vec4 pos = u_Projection * rotView * vec4(a_Position, 1.0);
    gl_Position = pos.xyww; // force depth = 1.0
}
)GLSL";

static const char* kSkybox_Frag = R"GLSL(
#version 450 core
in vec3 v_TexCoord;
out vec4 FragColor;
uniform samplerCube u_Cubemap;
uniform float u_Intensity;
void main() {
    vec3 col = texture(u_Cubemap, v_TexCoord).rgb * u_Intensity;
    col = pow(col, vec3(1.0/2.2)); // gamma
    FragColor = vec4(col, 1.0);
}
)GLSL";

static const char* kGrid_Vert = R"GLSL(
#version 450 core
layout(location=0) in vec3 a_Position;
layout(std140, binding=0) uniform Camera {
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProjection;
    vec4 u_CamPos;
};
out vec3 v_WorldPos;
out vec3 v_CamPos;
void main() {
    // Scale the quad to be very large
    vec3 pos = a_Position * 500.0;
    pos.y = 0.0;
    v_WorldPos = pos;
    v_CamPos = u_CamPos.xyz;
    gl_Position = u_ViewProjection * vec4(pos, 1.0);
}
)GLSL";

static const char* kGrid_Frag = R"GLSL(
#version 450 core
in vec3 v_WorldPos;
in vec3 v_CamPos;
out vec4 FragColor;
uniform float u_Scale;

float GridLine(float coord, float thickness) {
    float grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
    return 1.0 - min(grid, 1.0);
}

void main() {
    vec2 coord = v_WorldPos.xz / u_Scale;
    // Major grid (every 10 units)
    float majorLine = max(GridLine(coord.x * 0.1, 1.5), GridLine(coord.y * 0.1, 1.5)) * 0.6;
    // Minor grid (every 1 unit)
    float minorLine = max(GridLine(coord.x, 1.0), GridLine(coord.y, 1.0)) * 0.4;

    float line = max(majorLine, minorLine);

    // Fade with distance
    float dist = length(v_WorldPos.xz - v_CamPos.xz);
    float fade = 1.0 - smoothstep(40.0, 120.0, dist);

    // Axes: X=red, Z=blue
    float xAxis = GridLine(v_WorldPos.z / u_Scale, 2.0) * step(abs(v_WorldPos.z), 0.3);
    float zAxis = GridLine(v_WorldPos.x / u_Scale, 2.0) * step(abs(v_WorldPos.x), 0.3);

    vec3 color = vec3(line * 0.6);
    if (xAxis > 0.0) color = mix(color, vec3(0.86, 0.2, 0.2), xAxis);
    if (zAxis > 0.0) color = mix(color, vec3(0.2, 0.4, 0.86), zAxis);

    float alpha = max(line, max(xAxis, zAxis)) * fade;
    if (alpha < 0.01) discard;
    FragColor = vec4(color, alpha * 0.9);
}
)GLSL";

// ─── Skybox cube vertices ─────────────────────────────────────────────────────
static const float kSkyboxVerts[] = {
    -1,-1,-1,  1,-1,-1,  1, 1,-1, -1, 1,-1,
    -1,-1, 1,  1,-1, 1,  1, 1, 1, -1, 1, 1,
    -1, 1,-1,  1, 1,-1,  1, 1, 1, -1, 1, 1,
    -1,-1,-1,  1,-1,-1,  1,-1, 1, -1,-1, 1,
    -1,-1,-1, -1, 1,-1, -1, 1, 1, -1,-1, 1,
     1,-1,-1,  1, 1,-1,  1, 1, 1,  1,-1, 1,
};
static const uint32_t kSkyboxIdx[] = {
    0,1,2,2,3,0, 4,5,6,6,7,4, 8,9,10,10,11,8,
    12,13,14,14,15,12, 16,17,18,18,19,16, 20,21,22,22,23,20
};
// Grid quad
static const float kGridVerts[] = {
    -1,0,-1, 1,0,-1, 1,0,1, -1,0,1
};
static const uint32_t kGridIdx[] = { 0,1,2, 2,3,0 };

} // namespace R3D

// ─── Public interface ─────────────────────────────────────────────────────────

void Renderer3D::Init() {
    using namespace R3D;
    if (s_State.Initialized) return;

    // Compile shaders
    s_State.PBRShader    = BuildPBRShader();
    s_State.ShadowShader = BuildShadowShader();
    s_State.SkyboxShader = BuildSkyboxShader();
    s_State.GridShader   = BuildGridShader();

    // UBOs
    s_State.CameraUBO = UniformBuffer::Create(sizeof(CameraData), kCameraBinding);
    s_State.LightsUBO = UniformBuffer::Create(sizeof(LightsData), kLightsBinding);

    // Bind UBOs in shaders
    auto bindUBO = [&](std::shared_ptr<Shader> sh, const char* name, uint32_t point) {
        sh->Bind();
        uint32_t idx = glGetUniformBlockIndex(sh->GetID(), name);
        if (idx != GL_INVALID_INDEX) glUniformBlockBinding(sh->GetID(), idx, point);
    };
    bindUBO(s_State.PBRShader,    "Camera", kCameraBinding);
    bindUBO(s_State.PBRShader,    "Lights", kLightsBinding);
    bindUBO(s_State.SkyboxShader, "Camera", kCameraBinding);
    bindUBO(s_State.GridShader,   "Camera", kCameraBinding);

    // Shadow FBOs
    for (uint32_t i = 0; i < kMaxDirShadows; i++) {
        auto& sm = s_State.ShadowMaps[i];
        glGenFramebuffers(1, &sm.FBO);
        glGenTextures(1, &sm.Depth);
        glBindTexture(GL_TEXTURE_2D, sm.Depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F,
                     kShadowSize, kShadowSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float border[] = {1.0f,1.0f,1.0f,1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
        glBindFramebuffer(GL_FRAMEBUFFER, sm.FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sm.Depth, 0);
        glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Skybox VAO
    {
        s_State.SkyboxVAO = std::make_shared<VertexArray>();
        auto vbo = std::make_shared<VertexBuffer>(kSkyboxVerts, sizeof(kSkyboxVerts));
        vbo->SetLayout({{ ShaderDataType::Float3, "a_Position" }});
        s_State.SkyboxVAO->AddVertexBuffer(vbo);
        auto ibo = std::make_shared<IndexBuffer>(kSkyboxIdx, 36);
        s_State.SkyboxVAO->SetIndexBuffer(ibo);
    }

    // Grid VAO
    {
        s_State.GridVAO = std::make_shared<VertexArray>();
        auto vbo = std::make_shared<VertexBuffer>(kGridVerts, sizeof(kGridVerts));
        vbo->SetLayout({{ ShaderDataType::Float3, "a_Position" }});
        s_State.GridVAO->AddVertexBuffer(vbo);
        auto ibo = std::make_shared<IndexBuffer>(kGridIdx, 6);
        s_State.GridVAO->SetIndexBuffer(ibo);
    }

    s_State.Initialized = true;
    FL_ENGINE_INFO("Renderer3D initialized.");
}

void Renderer3D::Shutdown() {
    using namespace R3D;
    for (auto& sm : s_State.ShadowMaps) {
        glDeleteFramebuffers(1, &sm.FBO);
        glDeleteTextures(1, &sm.Depth);
    }
    s_State = State{};
    FL_ENGINE_INFO("Renderer3D shut down.");
}

void Renderer3D::BeginScene(const Camera& camera, const SceneLights& lights) {
    using namespace R3D;
    s_State.OpaqueQueue.clear();
    s_State.TransparentQueue.clear();
    s_State.Stats = {};

    // ── Camera UBO ────────────────────────────────────────────────────────
    s_State.Camera.View           = camera.GetViewMatrix();
    s_State.Camera.Projection     = camera.GetProjectionMatrix();
    s_State.Camera.ViewProjection = camera.GetProjectionMatrix() * camera.GetViewMatrix();
    s_State.Camera.Position       = glm::vec4(camera.GetPosition(), 1.0f);
    s_State.CameraUBO->SetData(&s_State.Camera, sizeof(CameraData));

    // ── Lights UBO ────────────────────────────────────────────────────────
    s_State.Lights = {};
    int nDir = std::min((int)lights.Directional.size(), (int)kMaxDirShadows);
    for (int i = 0; i < nDir; i++) {
        auto& src = lights.Directional[i];
        auto& dst = s_State.Lights.DirLights[i];
        dst.Direction    = glm::vec4(glm::normalize(src.Direction), src.Intensity);
        dst.Color        = glm::vec4(src.Color, src.CastShadow ? 1.0f : 0.0f);
        dst.ShadowParams = glm::vec4(src.ShadowBias, 0, 0, 0);

        // Build light-space matrix (ortho around camera position)
        glm::vec3 cp = camera.GetPosition();
        glm::mat4 lightProj = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 0.1f, 200.0f);
        glm::vec3 ld = glm::normalize(src.Direction);
        glm::mat4 lightView = glm::lookAt(cp - ld * 50.0f, cp, glm::vec3(0,1,0));
        s_State.LightSpaceMatrices[i] = lightProj * lightView;
        dst.LightSpaceMatrix = s_State.LightSpaceMatrices[i];
    }

    int nPt = std::min((int)lights.Point.size(), (int)kMaxPointLights);
    for (int i = 0; i < nPt; i++) {
        auto& src = lights.Point[i];
        auto& dst = s_State.Lights.PointLights[i];
        dst.Position    = glm::vec4(src.Position, src.Range);
        dst.ColorIntensity = glm::vec4(src.Color, src.Intensity);
        dst.Attenuation = glm::vec4(src.Constant, src.Linear, src.Quadratic, 0);
    }

    int nSp = std::min((int)lights.Spot.size(), (int)kMaxSpotLights);
    for (int i = 0; i < nSp; i++) {
        auto& src = lights.Spot[i];
        auto& dst = s_State.Lights.SpotLights[i];
        dst.Position  = glm::vec4(src.Position, src.Range);
        dst.Direction = glm::vec4(src.Direction, src.Intensity);
        dst.Color     = glm::vec4(src.Color, std::cos(glm::radians(src.OuterCutoff)));
        dst.Params    = glm::vec4(std::cos(glm::radians(src.InnerCutoff)), 0, 0, 0);
    }

    s_State.Lights.Ambient    = glm::vec4(lights.AmbientColor, lights.AmbientStrength);
    s_State.Lights.LightCounts = glm::ivec4(nDir, nPt, nSp, 0);
    s_State.LightsUBO->SetData(&s_State.Lights, sizeof(LightsData));
}

void Renderer3D::Submit(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, const glm::mat4& transform) {
    using namespace R3D;
    if (!mesh) return;

    DrawCall3D dc;
    dc.MeshPtr     = mesh;
    dc.MaterialPtr = material;
    dc.Transform   = transform;
    // Distance from camera (use translation component)
    glm::vec3 pos(transform[3]);
    glm::vec3 cp(s_State.Camera.Position);
    dc.DistanceSq  = glm::dot(pos - cp, pos - cp);
    // Transparent if material alpha < 1 (basic heuristic)
    dc.IsTransparent = false; // TODO[Renderer3D]: Implement material blend-mode handling to set IsTransparent based on material alpha or blend flags

    if (dc.IsTransparent)
        s_State.TransparentQueue.push_back(dc);
    else
        s_State.OpaqueQueue.push_back(dc);
}

void Renderer3D::EndScene() {
    using namespace R3D;

    // Sort opaque front-to-back
    std::sort(s_State.OpaqueQueue.begin(), s_State.OpaqueQueue.end(),
        [](const DrawCall3D& a, const DrawCall3D& b) { return a.DistanceSq < b.DistanceSq; });
    // Sort transparent back-to-front
    std::sort(s_State.TransparentQueue.begin(), s_State.TransparentQueue.end(),
        [](const DrawCall3D& a, const DrawCall3D& b) { return a.DistanceSq > b.DistanceSq; });

    // Shadow passes
    for (int i = 0; i < s_State.Lights.LightCounts.x; i++) {
        if (s_State.Lights.DirLights[i].Color.w > 0.5f) {
            // Minimal shadow pass
            glViewport(0, 0, kShadowSize, kShadowSize);
            glBindFramebuffer(GL_FRAMEBUFFER, s_State.ShadowMaps[i].FBO);
            glClear(GL_DEPTH_BUFFER_BIT);

            s_State.ShadowShader->Bind();
            s_State.ShadowShader->SetMat4("u_LightSpaceMatrix", s_State.LightSpaceMatrices[i]);

            for (auto& dc : s_State.OpaqueQueue) {
                s_State.ShadowShader->SetMat4("u_Model", dc.Transform);
                dc.MeshPtr->GetVAO()->Bind();  // bind before draw
                glDrawElements(GL_TRIANGLES, dc.MeshPtr->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            s_State.Stats.ShadowPasses++;
        }
    }

    // Restore viewport
    glViewport(0, 0, s_State.ViewportW, s_State.ViewportH);

    // Bind shadow maps to texture units 10..13
    for (int i = 0; i < (int)kMaxDirShadows; i++) {
        glActiveTexture(GL_TEXTURE10 + i);
        glBindTexture(GL_TEXTURE_2D, s_State.ShadowMaps[i].Depth);
    }
    s_State.PBRShader->Bind();
    for (int i = 0; i < (int)kMaxDirShadows; i++) {
        s_State.PBRShader->SetInt("u_ShadowMaps[" + std::to_string(i) + "]", 10 + i);
    }

    OpaquePass();
    TransparentPass();
}

void Renderer3D::OpaquePass() {
    using namespace R3D;
    s_State.PBRShader->Bind();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);

    for (auto& dc : s_State.OpaqueQueue) {
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(dc.Transform));
        s_State.PBRShader->SetMat4("u_Model", dc.Transform);
        s_State.PBRShader->SetMat4("u_NormalMatrix", normalMatrix);

        if (dc.MaterialPtr) dc.MaterialPtr->Bind();

        dc.MeshPtr->GetVAO()->Bind();
        glDrawElements(GL_TRIANGLES, dc.MeshPtr->GetIndexCount(), GL_UNSIGNED_INT, nullptr);

        if (dc.MaterialPtr) dc.MaterialPtr->Unbind();

        s_State.Stats.DrawCalls++;
        s_State.Stats.IndexCount += dc.MeshPtr->GetIndexCount();
    }
}

void Renderer3D::TransparentPass() {
    using namespace R3D;
    if (s_State.TransparentQueue.empty()) return;
    s_State.PBRShader->Bind();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    for (auto& dc : s_State.TransparentQueue) {
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(dc.Transform));
        s_State.PBRShader->SetMat4("u_Model", dc.Transform);
        s_State.PBRShader->SetMat4("u_NormalMatrix", normalMatrix);
        if (dc.MaterialPtr) dc.MaterialPtr->Bind();
        dc.MeshPtr->GetVAO()->Bind();
        glDrawElements(GL_TRIANGLES, dc.MeshPtr->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
        if (dc.MaterialPtr) dc.MaterialPtr->Unbind();
        s_State.Stats.DrawCalls++;
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void Renderer3D::DrawSkybox(uint32_t cubemapID) {
    using namespace R3D;
    glDepthFunc(GL_LEQUAL);
    s_State.SkyboxShader->Bind();
    s_State.SkyboxShader->SetInt("u_Cubemap", 0);
    s_State.SkyboxShader->SetFloat("u_Intensity", 1.0f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
    s_State.SkyboxVAO->Bind();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    glDepthFunc(GL_LESS);
}

void Renderer3D::DrawGrid(float scale) {
    using namespace R3D;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    s_State.GridShader->Bind();
    s_State.GridShader->SetFloat("u_Scale", scale);
    s_State.GridVAO->Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void Renderer3D::SetViewportSize(uint32_t w, uint32_t h) {
    R3D::s_State.ViewportW = w;
    R3D::s_State.ViewportH = h;
}

void Renderer3D::ResetStats() { R3D::s_State.Stats = {}; }
const RenderStats3D& Renderer3D::GetStats() { return R3D::s_State.Stats; }

// ─── Shader builders ─────────────────────────────────────────────────────────
std::shared_ptr<Shader> Renderer3D::BuildPBRShader() {
    return Shader::Create(R3D::kPBR_Vert, R3D::kPBR_Frag);
}
std::shared_ptr<Shader> Renderer3D::BuildShadowShader() {
    return Shader::Create(R3D::kShadow_Vert, R3D::kShadow_Frag);
}
std::shared_ptr<Shader> Renderer3D::BuildSkyboxShader() {
    return Shader::Create(R3D::kSkybox_Vert, R3D::kSkybox_Frag);
}
std::shared_ptr<Shader> Renderer3D::BuildGridShader() {
    return Shader::Create(R3D::kGrid_Vert, R3D::kGrid_Frag);
}

} // namespace Freely
