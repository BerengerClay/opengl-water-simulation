#version 450

in  vec3 FragPos;
in  vec3 Normal;
in  vec2 UV;
in  float FoamIntensity;

uniform vec3 viewPos;

out vec4 FragColor;

void main() {
    vec3 lightDir   = normalize(vec3(1.0, 1.0, 0.5));
    vec3 viewDir    = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, Normal);

    float diff  = max(dot(Normal, lightDir), 0.0);
    float spec  = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    float fres  = pow(1.0 - max(dot(viewDir, Normal), 0.0), 3.0);

    vec3 baseColor = vec3(0.0, 0.4, 0.8);
    vec3 specColor = vec3(1.0);
    vec3 fresColor = vec3(0.6, 0.8, 1.0);

    vec3 color = baseColor * diff
               + specColor * spec
               + fresColor * fres;

    vec3 foamColor = vec3(1.0);
    color = mix(color, foamColor, clamp(FoamIntensity, 0.0, 1.0));

    vec3 skyColor = vec3(0.4, 0.6, 1.0);
    float reflAmt = pow(clamp(Normal.y, 0.0, 1.0), 2.0);
    color = mix(color, skyColor, 0.4 * (1.0 - reflAmt));

    FragColor = vec4(color, 1.0);
}