#version 450

in vec3 WorldPos;

out vec4 FragColor;

void main() {
    float h = 0.5 + 0.5 * sin(WorldPos.x * 0.2) * cos(WorldPos.z * 0.2);
    vec3 grass = mix(vec3(0.2, 0.7, 0.2), vec3(0.4, 0.3, 0.1), h * 0.3);
    FragColor = vec4(grass, 1.0);
}