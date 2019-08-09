#version 450 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

// smooth out vec2 offset;
smooth out vec2 coords;

void main()
{
    vec2 offsets[4];
    offsets[0] = vec2(0.0f, 0.0f);
    offsets[1] = vec2(0.0f, 1.0f);
    offsets[2] = vec2(1.0f, 0.0f);
    offsets[3] = vec2(1.0f, 1.0f);

    for (int i = 0; i < 4; i++)
    {
        // offset = offsets[i];
        coords = offsets[i];

        // vec3 position = vec3(offset * 2.0f - 1.0f, 0.0f);
        vec3 position = vec3(coords * 2.0f - 1.0f, 0.0f);
        gl_Position = vec4(position, 1.0f);

        EmitVertex();
    }

    EndPrimitive();
}
