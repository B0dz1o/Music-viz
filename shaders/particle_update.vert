#version 330 core

// Input particle state
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aVelocity;
layout(location = 2) in float aLife;
layout(location = 3) in float aSize;

// Transform feedback outputs (captured by GPU, not rasterized)
out vec2  outPosition;
out vec2  outVelocity;
out float outLife;
out float outSize;

uniform float uDeltaTime;
uniform float uTime;
uniform float uBeatLevel;
uniform float uBassEnergy;
uniform float uMidEnergy;
uniform float uHighEnergy;

// Deterministic pseudo-random in [0,1]
float rand(vec2 seed) {
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    int id = gl_VertexID;

    float r1 = rand(vec2(float(id) * 1.1,  uTime));
    float r2 = rand(vec2(float(id) * 2.3,  uTime + 0.7));
    float r3 = rand(vec2(float(id) * 3.7,  uTime + 1.4));

    if (aLife <= 0.0) {
        // ---- Respawn particle ----
        float angle      = r1 * 6.28318530718;
        float emitR      = 0.03 + r2 * 0.08;
        // Speed surges on beat, also scales with bass energy
        float speed      = 0.12 + r3 * 0.22 + uBeatLevel * 0.7 + uBassEnergy * 0.25;

        outPosition = vec2(cos(angle) * emitR, sin(angle) * emitR);
        outVelocity = vec2(cos(angle) * speed,  sin(angle) * speed);
        outLife     = 0.35 + r2 * 0.65;
        outSize     = 3.0  + r3 * 9.0 + uBeatLevel * 7.0;
    } else {
        // ---- Integrate physics ----
        vec2 dir = normalize(aVelocity + vec2(0.0001, 0.0)); // epsilon avoids zero-vector normalisation

        // Constant downward drift (gravity)
        vec2 gravity = vec2(0.0, -0.06);

        // Beat delivers a strong radial impulse
        vec2 beatImpulse = dir * uBeatLevel * 0.9;

        // High-frequency energy adds turbulence
        vec2 turbulence = vec2(
            rand(vec2(float(id), uTime * 12.0)) * 2.0 - 1.0,
            rand(vec2(float(id) + 77.0, uTime * 12.0)) * 2.0 - 1.0
        ) * uHighEnergy * 0.12;

        vec2 acc     = (gravity + beatImpulse + turbulence);
        vec2 newVel  = aVelocity + acc * uDeltaTime;
        // Velocity damping
        newVel *= (1.0 - 0.75 * uDeltaTime);

        outPosition = aPosition + newVel * uDeltaTime;
        outVelocity = newVel;
        // Lifetime drains faster with mid-range energy
        outLife     = aLife - uDeltaTime * (0.28 + uMidEnergy * 0.22);
        outSize     = aSize;
    }
}
