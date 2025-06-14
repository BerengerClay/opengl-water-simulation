#version 330 core

in vec3 fragPos;
in vec3 fragNormal;

out vec4 FragColor;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.0, 0.5));  // direction de la lumière
    vec3 normal = normalize(fragNormal);

    // Éclairage Lambertien (diffus)
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 baseColor = vec3(0.0, 0.4, 0.8);  // couleur de l'eau
    vec3 finalColor = baseColor * diff;

    FragColor = vec4(finalColor, 1.0);
}
