#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D heightMap;
uniform int     gridSize;
uniform float   step;
uniform float   heightScale;  // <-- nouveau

out vec3 FragPos;
out vec3 Normal;

void main() {
    // coordonnées UV [0,1]
    vec2 uv = (aPos.xz / (step * gridSize)) + 0.5;

    // récupère la hauteur (peut être négative)
    float h = texture(heightMap, uv).r;
    // applique l'échelle
    vec3 pos = vec3(aPos.x, h * heightScale, aPos.z);

    // calcul normal (approx par differences)
    float du = 1.0 / float(gridSize);
    float hl = texture(heightMap, uv + vec2(-du,0)).r;
    float hr = texture(heightMap, uv + vec2( du,0)).r;
    float hd = texture(heightMap, uv + vec2(0,-du)).r;
    float hu = texture(heightMap, uv + vec2(0, du)).r;
    vec3 n = normalize(vec3((hl-hr)*heightScale, 2.0*step, (hd-hu)*heightScale));

    FragPos = vec3(model * vec4(pos, 1.0));
    Normal  = mat3(transpose(inverse(model))) * n;
    gl_Position = projection * view * model * vec4(pos,1.0);
}
