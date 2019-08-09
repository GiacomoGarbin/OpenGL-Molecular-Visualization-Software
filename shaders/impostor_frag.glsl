#version 450

layout (location = 0) out vec3 center;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec3 albedo;
layout (location = 3) out vec3 colour;

// layout (location = 4) out vec3 silhouette;

struct ImpostorData
{
    vec3 center;
    vec3 normal;
};

void SetImpostorData(out ImpostorData impostor);

in FragData
{
    flat vec3 center;
    flat float radius;
    flat vec3 albedo;
    flat vec2 number;
    flat vec3 colour;
    smooth vec2 offset;
} frag;

// uniform mat4 model;
// uniform mat4 view;
uniform mat4 projection;

uniform bool grayscale;

void main()
{
    ImpostorData impostor;
    SetImpostorData(impostor);

    vec4 clip = projection * vec4(impostor.center, 1.0f);
    float depth = clip.z / clip.w;
    gl_FragDepth = ((gl_DepthRange.diff * depth) + gl_DepthRange.near + gl_DepthRange.far) * 0.5f;

    center = impostor.center;
    normal = impostor.normal;
    albedo = frag.albedo;
    colour = frag.colour;

    /* ####################### PRETTY ALBEDO ####################### */

    // grayscale weights
    vec3 weight = vec3(0.2126f, 0.7152f, 0.0722f);
    float shade = dot((impostor.normal + 1.0f) * 0.5f, weight);
    // shade power
    float power = 0.1f;
    // albedo = frag.albedo * shade + (1.0f - shade) * power;
    // albedo = (impostor.normal + 1.0f) * 0.5f;

    if (grayscale)
    {
        albedo = vec3(dot(albedo, weight));
    }
}

/*
void SetImpostorData(out ImpostorData impostor)
{
    // center of the sphere (atom) in view space
    vec3 center = vec3(view * model * vec4(frag.center, 1.0f));
    // we shoot a ray from the camera position (0 in view space) to the frag position on the quad
    vec3 ray = normalize(center + vec3(frag.offset * frag.radius, 0.0f));

    float b = 2.0f * dot(ray, -center);
    float c = dot(center, center) - (frag.radius * frag.radius);

    float delta = (b * b) - (4.0f * c);

    if(delta < 0.0f)
    {
        discard;
    }

    delta = sqrt(delta);
    float t1 = (-b + delta) * 0.5f;
    float t2 = (-b - delta) * 0.5f;

    float t = min(t1, t2);
    // frag position on the sphere in view space
    impostor.center = ray * t;
    impostor.normal = normalize(impostor.center - center);
}
*/

void SetImpostorData(out ImpostorData impostor)
{
    // we shoot a ray from the camera position (0 in view space) to the frag position on the quad
    vec3 ray = normalize(frag.center + vec3(frag.offset * frag.radius, 0.0f));

    float b = 2.0f * dot(ray, -frag.center);
    float c = dot(frag.center, frag.center) - (frag.radius * frag.radius);

    float delta = (b * b) - (4.0f * c);

    if(delta < 0.0f)
    {
        discard;
    }

    delta = sqrt(delta);
    float t1 = (-b + delta) * 0.5f;
    float t2 = (-b - delta) * 0.5f;

    float t = min(t1, t2);
    // frag position on the sphere in view space
    impostor.center = ray * t;
    impostor.normal = normalize(impostor.center - frag.center);
}
