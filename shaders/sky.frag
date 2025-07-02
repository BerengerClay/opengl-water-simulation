#version 330 core
in vec2 UV;
out vec4 FragColor;
void main() {
    vec3 top = vec3(0.45, 0.7, 1.0);
    vec3 bot = vec3(0.8, 0.95, 1.0);
    FragColor = vec4(mix(bot, top, UV.y), 1.0);
}