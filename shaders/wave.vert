#version 330 core

layout(location = 0) in vec3 position;

uniform float time;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int NUM_WAVES = 3;
uniform float amplitude[NUM_WAVES];
uniform float frequency[NUM_WAVES];
uniform float speed[NUM_WAVES];
uniform float steepness[NUM_WAVES];
uniform vec2 direction[NUM_WAVES];

out vec3 fragPos;
out vec3 fragNormal;

void main() {
    vec3 pos = position;
    vec3 normal = vec3(0.0, 1.0, 0.0);
    vec2 displacement = vec2(0.0);
    float height = 0.0;

    for (int i = 0; i < NUM_WAVES; ++i) {
        float phase = dot(direction[i], pos.xz) * frequency[i] + time * speed[i];
        float wave = sin(phase);
        float cosWave = cos(phase);

        // Déplacement latéral et vertical
        displacement += direction[i] * (steepness[i] * amplitude[i] * cosWave);
        height += amplitude[i] * wave;

        // Normale approximée
        normal.x += -direction[i].x * frequency[i] * amplitude[i] * cosWave;
        normal.y += steepness[i] * frequency[i] * amplitude[i] * sin(phase);
        normal.z += -direction[i].y * frequency[i] * amplitude[i] * cosWave;
    }

    pos.xz += displacement;
    pos.y += height;

    fragNormal = mat3(transpose(inverse(model))) * normalize(normal);
    fragPos = vec3(model * vec4(pos, 1.0));
    gl_Position = projection * view * vec4(fragPos, 1.0);
}
