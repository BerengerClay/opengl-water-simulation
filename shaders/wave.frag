#version 330 core

in vec3 fragPos;
in vec3 fragNormal;

out vec4 FragColor;

uniform vec3 viewPos;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.0, 0.8));
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    // Éclairage
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    float fresnel = pow(1.0 - max(dot(viewDir, normal), 0.0), 3.0);

    // Couleurs
    vec3 deepColor = vec3(0.0, 0.1, 0.2);
    vec3 shallowColor = vec3(0.0, 0.5, 0.9);
    float depthFade = clamp((fragPos.y + 1.5) / 3.0, 0.0, 1.0); // assombrir selon la hauteur
    vec3 baseColor = mix(deepColor, shallowColor, depthFade);

    vec3 highlight = vec3(0.7, 0.8, 1.0); // effet fresnel
    vec3 finalColor = baseColor * diff + vec3(1.0) * spec + highlight * fresnel;

    FragColor = vec4(finalColor, 1.0); // tu peux mettre < 1.0 pour simuler la transparence
}
