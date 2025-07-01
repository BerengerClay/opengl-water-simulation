#version 330 core
in  vec3 FragPos;
in  vec3 Normal;
in vec2 UV;
uniform float threshold1;
uniform float threshold2;
out vec4 FragColor;

uniform vec3 viewPos;
uniform sampler2D velocityMap;
uniform float velocityScale; // to control sensitivity
uniform float dropProgress;

void main() {
    vec3 lightDir  = normalize(vec3(1.0, 1.0, 0.5));
    vec3 viewDir   = normalize(viewPos - FragPos);
    vec3 reflectDir= reflect(-lightDir, Normal);

    float diff    = max(dot(Normal, lightDir), 0.0);
    float spec    = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    float fresnel = pow(1.0 - max(dot(viewDir, Normal), 0.0), 3.0);

    vec3 baseColor  = vec3(0.0, 0.4, 0.8);
    vec3 specColor  = vec3(1.0);
    vec3 fresColor  = vec3(0.6, 0.8, 1.0);

    // --- FOAM: based on slope ---
    float velocity = texture(velocityMap, UV).r;
    float foam = smoothstep(threshold1, threshold2, velocity * velocityScale);
    vec3 foamColor = vec3(1.0);


    //vec3 color = mix(baseColor * diff + specColor * spec + fresColor * fresnel, foamColor, foam);
    vec3 color = baseColor * diff + specColor * spec + fresColor * fresnel;

    vec3 skyColor = vec3(0.4, 0.6, 1.0);
    float reflectAmount = pow(clamp(Normal.y, 0.0, 1.0), 2.0);
    color = mix(color, skyColor, 0.4 * (1.0 - reflectAmount));

    FragColor = vec4(color, 0.6);
}
