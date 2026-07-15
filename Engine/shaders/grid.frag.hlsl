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
