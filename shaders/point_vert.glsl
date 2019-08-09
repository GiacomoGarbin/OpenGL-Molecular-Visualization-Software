#version 450 core

layout (location = 0) in vec3 center;
layout (location = 1) in float radius;
layout (location = 2) in vec3 albedo;
// layout (location = 3) in int number;
// layout (location = 3) in uint number;
// layout (location = 3) in float number;
layout (location = 3) in vec2 number;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform int OutlineMode;

out FragData
{
    flat vec3 albedo;
    flat vec2 number;
} frag;

void main()
{
    mat4 MVP = projection * view * model;
    gl_Position = MVP * vec4(center, 1.0f);

    /*
    uvec3 mask = uvec3(0x00FF0000, 0x0000FF00, 0x000000FF);
    uvec3 shift = uvec3(16, 8, 0);

    // vec3 colour = ((uvec3(number) & mask) >> shift) / 255.0f;
    vec3 colour = ((uvec3(number[mode ? 0 : 1]) & mask) >> shift) / 255.0f;

    // atoms
    if (mode)
    {
        // ligand
        if ((1733 <= number[mode ? 0 : 1]) && (number[mode ? 0 : 1] <= 1752))
        {
            colour = vec3(0.0f, 1.0f, 0.0f);
        }

        // carbon
        if (number[mode ? 0 : 1] == 1753)
        {
            colour = vec3(1.0f, 1.0f, 0.0f);
        }
    }
    // residues
    else
    {
        // ligand
        if (number[mode ? 0 : 1] == 120)
        {
            colour = vec3(0.0f, 1.0f, 1.0f);
        }

        // carbon
        if (number[mode ? 0 : 1] == 121)
        {
            colour = vec3(1.0f, 0.0f, 1.0f);
        }
    }
    */

    // frag.albedo = colour;
    frag.number = number;
}
