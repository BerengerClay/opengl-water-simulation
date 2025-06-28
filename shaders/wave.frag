#version 330 core
in  vec3 FragPos;
in  vec3 Normal;
out vec4 FragColor;

uniform vec3 viewPos;

void main() {
    vec3 lightDir  = normalize(vec3(1.0, 1.0, 0.5));
    vec3 viewDir   = normalize(viewPos - FragPos);
    vec3 reflectDir= reflect(-lightDir, Normal);

    // Lambertien + Phong spéculaire + Fresnel
    float diff    = max(dot(Normal, lightDir), 0.0);
    float spec    = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    float fresnel = pow(1.0 - max(dot(viewDir, Normal), 0.0), 3.0);

    vec3 baseColor  = vec3(0.0, 0.4, 0.8);
    vec3 specColor  = vec3(1.0);
    vec3 fresColor  = vec3(0.6, 0.8, 1.0);

    vec3 color = baseColor * diff + specColor * spec + fresColor * fresnel;
    FragColor = vec4(color, 0.6);
}
