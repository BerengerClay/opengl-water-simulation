#version 330 core
out vec2 UV;
void main() {
    vec2 pos[4] = vec2[](
        vec2(-1, -1),
        vec2( 1, -1),
        vec2(-1,  1),
        vec2( 1,  1)
    );
    UV = (pos[gl_VertexID] + 1.0) * 0.5;
    gl_Position = vec4(pos[gl_VertexID], 0, 1);
}