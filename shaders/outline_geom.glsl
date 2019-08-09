#version 450 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

out FragData
{
    smooth vec2 coords;
} frag;

void main()
{
    vec2 coords[4];
    coords[0] = vec2(0.0f, 0.0f);
    coords[1] = vec2(0.0f, 1.0f);
    coords[2] = vec2(1.0f, 0.0f);
    coords[3] = vec2(1.0f, 1.0f);

    for (int i = 0; i < 4; i++)
    {
        frag.coords = coords[i];

        vec3 position = vec3(coords[i] * 2.0f - 1.0f, 0.0f);
        gl_Position = vec4(position, 1.0f);

        gl_PrimitiveID = gl_PrimitiveIDIn;

        EmitVertex();
    }

    EndPrimitive();
}
