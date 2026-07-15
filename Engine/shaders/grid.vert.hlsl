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
