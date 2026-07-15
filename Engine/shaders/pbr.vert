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
