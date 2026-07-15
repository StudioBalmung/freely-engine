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
