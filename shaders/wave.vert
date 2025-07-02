// shaders/wave.vert
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D heightMap;
uniform sampler2D foamMap;    // <-- nouvelle texture de mousse
uniform int     gridSize;
uniform float   step;
uniform float   heightScale;

out vec3 FragPos;
out vec3 Normal;
out vec2 UV;
out float FoamIntensity;

void main() {
    // UV normalisées [0,1] sur la grille
    UV = aPos.xz / (step * gridSize) + 0.5;

    // Hauteur de l'eau
    float h = texture(heightMap, UV).r;
    vec3 pos = vec3(aPos.x, h * heightScale, aPos.z);

    // Calcul approximatif de la normale par différences finies
    float du = 1.0 / float(gridSize);
    float hl = texture(heightMap, UV + vec2(-du, 0)).r;
    float hr = texture(heightMap, UV + vec2( du, 0)).r;
    float hd = texture(heightMap, UV + vec2(0, -du)).r;
    float hu = texture(heightMap, UV + vec2(0,  du)).r;
    vec3 n = normalize(vec3((hl - hr) * heightScale,
                             2.0 * step,
                            (hd - hu) * heightScale));

    FragPos       = vec3(model * vec4(pos, 1.0));
    Normal        = mat3(transpose(inverse(model))) * n;
    FoamIntensity = texture(foamMap, UV).r;  // lecture de la mousse

    gl_Position = projection * view * model * vec4(pos, 1.0);
}