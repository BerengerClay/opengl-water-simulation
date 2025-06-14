#version 330 core

layout(location = 0) in vec3 position;

uniform float time;
uniform float amplitude;
uniform float frequency;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragPos;
out vec3 fragNormal;

void main() {
    // Position animée (hauteur)
    vec3 pos = position;
    float wave = amplitude * sin(frequency * (pos.x + pos.z) + time);
    pos.y += wave;

    // Calcul analytique de la normale via gradient de la sinusoïde
    float dWave_dx = amplitude * frequency * cos(frequency * (pos.x + pos.z) + time);
    float dWave_dz = amplitude * frequency * cos(frequency * (pos.x + pos.z) + time);

    // Les dérivées partielles donnent le vecteur tangents dans x et z
    vec3 tangentX = normalize(vec3(1.0, dWave_dx, 0.0));
    vec3 tangentZ = normalize(vec3(0.0, dWave_dz, 1.0));
    vec3 normal = normalize(cross(tangentZ, tangentX)); // produit vectoriel des tangentes

    // Transformation et passage au fragment shader
    fragNormal = mat3(transpose(inverse(model))) * normal;
    fragPos = vec3(model * vec4(pos, 1.0));

    gl_Position = projection * view * vec4(fragPos, 1.0);
}
