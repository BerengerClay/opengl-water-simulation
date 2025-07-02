#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 model, view, projection;
out vec3 WorldPos;
void main() {
    float dist = length(aPos.xz);
    float base = -5.0; // base elevation
    float slope = 0.08; // how much higher per unit distance
    float height = base + slope * (dist - 0.0); // 0.0 can be waterRadius if you want
    vec3 elevated = aPos + vec3(0.0, height, 0.0);
    WorldPos = elevated;
    gl_Position = projection * view * model * vec4(elevated, 1.0);
}