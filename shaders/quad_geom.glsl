#version 450 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

out FragData
{
    smooth vec2 offset;
} frag;

void main()
{
    vec2 offset[4];
    offset[0] = vec2(0.0f, 0.0f);
    offset[1] = vec2(0.0f, 1.0f);
    offset[2] = vec2(1.0f, 0.0f);
    offset[3] = vec2(1.0f, 1.0f);

    for (int i = 0; i < 4; i++)
    {
        frag.offset = offset[i];

        vec3 position = vec3(offset[i] * 2.0f - 1.0f, 0.0f);
        gl_Position = vec4(position, 1.0f);

        EmitVertex();
    }

    EndPrimitive();
}
