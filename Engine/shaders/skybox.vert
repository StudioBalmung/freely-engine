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
