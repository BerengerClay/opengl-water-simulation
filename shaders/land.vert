#version 450

layout(location=0) in vec3 aPos;

uniform mat4 model, view, projection;

out vec3 WorldPos;

void main() {
    float dist = length(aPos.xz);
    float base = -5.0;
    float slope = 0.08;
    float height = base + slope * dist;
    vec3 elevated = aPos + vec3(0.0, height, 0.0);
    WorldPos = elevated;
    gl_Position = projection * view * model * vec4(elevated, 1.0);
}