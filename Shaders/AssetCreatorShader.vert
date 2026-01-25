#version 460
layout(location = 0) out vec2 fragTexCoord;

void main()
{
    float u = -1.0;
    float v = -1.0;
    if (gl_VertexIndex == 1) u = 3.0;
    if (gl_VertexIndex == 2) v = 3.0;

    gl_Position = vec4(u, v, 0.0, 1.0);
    fragTexCoord = (vec2(u, v) * 0.5) + 0.5;
}