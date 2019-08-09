#version 450 core

layout (location = 0) in vec3 center;
layout (location = 1) in float radius;
layout (location = 2) in vec3 albedo;
layout (location = 3) in vec2 number;

uniform int OutlineMode;

out VertData
{
    vec3 center;
    float radius;
    // vec3 albedo;
    vec2 number;
    vec3 colour;
} vert;

void main()
{
    uvec3 mask = uvec3(0x00FF0000, 0x0000FF00, 0x000000FF);
    uvec3 shift = uvec3(16, 8, 0);

    vec3 colour;

    switch (OutlineMode)
    {
    // residue number
    case 0:
    case 1:
    {
        colour = ((uvec3(number[1]) & mask) >> shift) / 255.0f;
        break;
    }
    // atom number
    case 2:
    case 3:
    {
        colour = ((uvec3(number[0]) & mask) >> shift) / 255.0f;
        break;
    }
    }

    vert.center = center;
    vert.radius = radius;
    // vert.albedo = albedo;
    vert.number = number;
    vert.colour = colour;
}
