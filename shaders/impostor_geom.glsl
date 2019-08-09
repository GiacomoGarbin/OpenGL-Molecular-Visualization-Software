#version 450 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in VertData
{
    vec3 center;
    float radius;
    vec3 albedo;
    vec2 number;
    vec3 colour;
} vert[];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out FragData
{
    flat vec3 center;
    flat float radius;
    flat vec3 albedo;
    flat vec2 number;
    flat vec3 colour;
    smooth vec2 offset;
} frag;

void main()
{
    vec2 offsets[4];
    offsets[0] = vec2(-1.0f, -1.0f);
    offsets[1] = vec2(-1.0f, +1.0f);
    offsets[2] = vec2(+1.0f, -1.0f);
    offsets[3] = vec2(+1.0f, +1.0f);

    float BoxCorrection = 1.5f;

    for (int i = 0; i < 4; i++)
    {
        gl_Position = view * model * vec4(vert[0].center, 1.0f);

        // frag.center = vert[0].center;
        frag.center = vec3(gl_Position);
        frag.radius = vert[0].radius;
        frag.albedo = vert[0].albedo;
        frag.number = vert[0].number;
        frag.colour = vert[0].colour;
        frag.offset = offsets[i] * BoxCorrection;

        // gl_Position = view * model * vec4(frag.center, 1.0f);
        gl_Position.xy += (frag.offset * frag.radius);
        gl_Position = projection * gl_Position;

        gl_PrimitiveID = gl_PrimitiveIDIn;

        EmitVertex();
    }

    EndPrimitive();
}
