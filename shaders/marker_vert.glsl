#version 450 core

struct Light
{
    // geometry
    vec3 position;
    vec3 direction;
    mat4 model;
    // properties
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    // attenuation
    float constant;
    float linear;
    float quadratic;
};

uniform Light light;

// uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragColor;

void main()
{
    vec3 vertices[14];
    vertices[0x0] = vec3(-1.0f, +1.0f, +1.0f);
    vertices[0x1] = vec3(+1.0f, +1.0f, +1.0f);
    vertices[0x2] = vec3(-1.0f, -1.0f, +1.0f);
    vertices[0x3] = vec3(+1.0f, -1.0f, +1.0f);
    vertices[0x4] = vec3(+1.0f, -1.0f, -1.0f);
    vertices[0x5] = vec3(+1.0f, +1.0f, +1.0f);
    vertices[0x6] = vec3(+1.0f, +1.0f, -1.0f);
    vertices[0x7] = vec3(-1.0f, +1.0f, +1.0f);
    vertices[0x8] = vec3(-1.0f, +1.0f, -1.0f);
    vertices[0x9] = vec3(-1.0f, -1.0f, +1.0f);
    vertices[0xA] = vec3(-1.0f, -1.0f, -1.0f);
    vertices[0xB] = vec3(+1.0f, -1.0f, -1.0f);
    vertices[0xC] = vec3(-1.0f, +1.0f, -1.0f);
    vertices[0xD] = vec3(+1.0f, +1.0f, -1.0f);

    mat4 MVP = projection * view * light.model;
    vec3 position = light.position + vertices[gl_VertexID] * 1.0f;
    gl_Position = MVP * vec4(position, 1.0f);

    FragColor = (vertices[gl_VertexID] + 1.0f) * 0.5f;
    FragColor = light.diffuse * 0.75f + FragColor * 0.25f;
}
