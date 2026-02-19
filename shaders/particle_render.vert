#version 330 core

layout(location = 0) in vec2  aPosition;
layout(location = 1) in vec2  aVelocity;
layout(location = 2) in float aLife;
layout(location = 3) in float aSize;

out float vLife;
out vec3  vColor;

uniform float uBeatLevel;
uniform float uTime;
uniform float uBassEnergy;
uniform float uHighEnergy;

void main() {
    gl_Position  = vec4(aPosition, 0.0, 1.0);
    gl_PointSize = aSize * (0.3 + aLife * 0.7);
    vLife = aLife;

    // Hue drifts over time; vertex ID spreads particles across the spectrum.
    // 0.00613 ≈ golden-ratio-like step (avoids repeating patterns across 5000 particles)
    // 0.07 = slow time-based hue rotation speed
    // 0.4  = how strongly bass energy shifts the palette
    float hue = fract(float(gl_VertexID) * 0.00613 + uTime * 0.07 + uBassEnergy * 0.4);
    float h6  = hue * 6.0;
    float r   = clamp(abs(h6 - 3.0) - 1.0, 0.0, 1.0);
    float g   = clamp(2.0 - abs(h6 - 2.0), 0.0, 1.0);
    float b   = clamp(2.0 - abs(h6 - 4.0), 0.0, 1.0);
    // Desaturate slightly toward white
    vec3 rgb  = mix(vec3(1.0), vec3(r, g, b), 0.75);
    // Brightness pulse on beat + high-freq sparkle
    float bright = 0.55 + uBeatLevel * 0.35 + uHighEnergy * 0.10;
    vColor = rgb * bright;
}
