#version 450

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
    // flat vec3 albedo;
    flat vec3 colour;
    smooth vec2 offset;
} frag;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// out float silhouette;
out vec3 silhouette;

void main()
{
    ImpostorData impostor;
    SetImpostorData(impostor);

    vec4 clip = projection * vec4(impostor.center, 1.0f);
    float depth = clip.z / clip.w;
    gl_FragDepth = ((gl_DepthRange.diff * depth) + gl_DepthRange.near + gl_DepthRange.far) * 0.5f;

    // silhouette = 0.0f;
    silhouette = frag.colour;
}

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
