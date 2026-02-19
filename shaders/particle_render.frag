#version 330 core

in float vLife;
in vec3  vColor;

out vec4 FragColor;

void main() {
    // gl_PointCoord is in [0,1]x[0,1]; map to [-0.5,0.5] circle
    vec2  coord = gl_PointCoord - vec2(0.5);
    float dist  = length(coord);
    if (dist > 0.5) discard;

    // Soft Gaussian-like fade toward edges
    float softness = 1.0 - smoothstep(0.15, 0.5, dist);
    float alpha    = vLife * softness;

    FragColor = vec4(vColor, alpha);
}
