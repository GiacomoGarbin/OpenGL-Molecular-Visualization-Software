#version 450 core

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light
{
    // geometry
    vec3 position;
    vec3 direction;

    bool free;
    mat4 model;

    // properties
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    // attenuation
    bool attenuation;
    float constant;
    float linear;
    float quadratic;
};

struct ScreenSpaceAmbientOcclusion
{
    bool on;
    sampler2D sampler;
};

struct FragData
{
    vec3 center;
    vec3 normal;
    vec3 albedo;
    float SSAO;
    Material material;
};

void SetFragData(out FragData frag);
vec3 GetLighting(Light light, FragData frag, vec3 eye);
vec3 GetLighting2(Light light, FragData frag);
vec3 GetLighting3(Light light, FragData frag, vec3 eye);

vec3 lerp(vec3 a, vec3 b, float t);
vec3 GetColor(vec3 albedo, float value, float size);

smooth in vec2 offset;

uniform mat4 view;

uniform sampler2D center;
uniform sampler2D normal;
uniform sampler2D albedo;

uniform ScreenSpaceAmbientOcclusion SSAO;
uniform Light light;

uniform vec4 factors;
uniform float shininess;

out vec4 PixelColor;

void main()
{
    FragData frag;
    SetFragData(frag);

    /*
    Light light;
    light.ambient = vec3(0.25f);
    light.diffuse = vec3(0.5f);
    light.specular = vec3(0.75f);
    */

    if (frag.albedo == vec3(0.0f))
    {
        discard;
    }

    // we work in view space, so eye position is vec3(0)
    // PixelColor = vec4(GetLighting(light, frag, vec3(0.0f)), 1.0f);
    // PixelColor = vec4(GetLighting2(light, frag), 1.0f);
    PixelColor = vec4(GetLighting3(light, frag, vec3(0.0f)), 1.0f);

    /*
    PixelColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
    PixelColor = vec4(offset, 0.0f, 1.0f);
    PixelColor = vec4(frag.albedo, 1.0f);
    PixelColor = vec4(light.specular, 1.0f);
    */
}

vec3 lerp(vec3 a, vec3 b, float t)
{
    vec3 c;
    t = clamp(t, 0.0f, 1.0f);

    for (int i = 0; i < 3; i++)
    {
        c[i] = a[i] * (1.0f - t) + b[i] * t;
    }

    return c;
}

vec3 GetColor(vec3 albedo, float value, float size)
{
    if (value < 0)
    {
        float t = (size + value) / size;
        return lerp(vec3(0.0f), albedo, t);
    }

    if (value > 0)
    {
        float t = (size - value) / size;
        return lerp(vec3(1.0f), albedo, t);
    }

    return albedo;
}

void SetFragData(out FragData frag)
{
    frag.center = texture(center, offset).xyz;
    // frag.normal = normalize(texture(normal, offset).xyz);
    frag.normal = texture(normal, offset).xyz;
    frag.albedo = texture(albedo, offset).rgb;

    /*
    if (light.attenuation)
    {
        if (frag.albedo.x > 0.0f && frag.albedo.x <= 1.0f)
        {
            frag.albedo = vec3(1.0f);
        }
    }
    */

    if (SSAO.on)
    {
       frag.SSAO = texture(SSAO.sampler, offset).r;
    }

    /*
    float AmbientFactor = 1.0f;
    float DiffuseFactor = 1.0f;
    float SpecularFactor = 1.0f;
    float shininess = 32.0f;

    Material material = Material
    (
        frag.albedo * AmbientFactor,
        frag.albedo * DiffuseFactor,
        frag.albedo * SpecularFactor,
        shininess
    );
    */

    float size = factors.w;
    // float shininess = 32.0f;

    Material material = Material
    (
        GetColor(frag.albedo, factors.x, size),
        GetColor(frag.albedo, factors.y, size),
        GetColor(frag.albedo, factors.z, size),
        shininess
    );

    frag.material = material;
}

vec3 GetLighting(Light light, FragData frag, vec3 eye)
{
    // ambient
    vec3 ambient = frag.material.ambient * light.ambient;

    if (SSAO.on)
    {
        ambient *= frag.SSAO;
    }

    float factor;

    // light position in view space
    // vec3 p = vec3(view * light.model * vec4(light.position, 1.0f));
    // vec3 p = vec3(view * vec4(light.position, 1.0f));
    vec3 position = vec3(0.0f);

    // diffuse
    vec3 direction = normalize(position - frag.center);
    factor = max(dot(frag.normal, direction), 0.0f);
    vec3 diffuse = factor * frag.material.diffuse * light.diffuse;

    // specular
    vec3 halfway = normalize(direction + normalize(eye - frag.center));
    factor = pow(max(dot(frag.normal, halfway), 0.0f), frag.material.shininess);
    vec3 specular = factor * frag.material.specular * light.specular;

    // attenuation
    if (light.attenuation)
    {
        float distance = length(position - frag.center);
        float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
    }

    return ambient + diffuse + specular;
}

vec3 GetLighting2(Light light, FragData frag)
{
    /*
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
    float AmbientOcclusion = texture(ssao, TexCoords).r;
    */

    light.diffuse = vec3(1.0f);
    light.specular = vec3(1.0f);

    // then calculate lighting as usual
    // vec3 ambient = vec3(0.3f * frag.albedo * frag.SSAO);
    // vec3 ambient = vec3(0.3f * frag.albedo);
    vec3 ambient = vec3(0.75f * frag.albedo);

    if (SSAO.on)
    {
        ambient *= frag.SSAO;
    }

    vec3 lighting  = ambient;
    vec3 viewDir  = normalize(-frag.center); // viewpos is (0.0.0)
    // diffuse
    vec3 lightDir = normalize(light.position - frag.center);
    vec3 diffuse = max(dot(frag.normal, lightDir), 0.0) * frag.albedo * light.diffuse; // glm::vec3(0.2, 0.2, 0.7);
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(frag.normal, halfwayDir), 0.0), 8.0);
    vec3 specular = light.specular * spec;
    // attenuation
    if (light.attenuation)
    {
        float distance = length(light.position - frag.center);
        float attenuation = 1.0 / (1.0 + light.linear * distance + light.quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }

    return lighting * 1.1f;
}

vec3 GetLighting3(Light light, FragData frag, vec3 eye)
{
    light.ambient = vec3(1.0f);
    light.diffuse = vec3(1.0f);
    light.specular = vec3(1.0f);

    // ambient
    vec3 ambient = frag.material.ambient * light.ambient;

    if (SSAO.on)
    {
        ambient *= frag.SSAO;
    }

    float factor;

    // light position in view space
    // vec3 p = vec3(view * light.model * vec4(light.position, 1.0f));
    // vec3 p = vec3(view * vec4(light.position, 1.0f));
    // vec3 position = vec3(0.0f);
    vec3 position = light.free ? vec3(view * light.model * vec4(light.position, 1.0f)) : vec3(0.0f);

    if (light.free)
    {
        // return vec3(view * light.model * vec4(light.position, 1.0f));
    }

    // diffuse
    vec3 direction = normalize(position - frag.center);
    factor = max(dot(frag.normal, direction), 0.0f);
    vec3 diffuse = factor * frag.material.diffuse * light.diffuse;

    // specular
    vec3 halfway = normalize(direction + normalize(eye - frag.center));
    factor = pow(max(dot(frag.normal, halfway), 0.0f), frag.material.shininess);
    vec3 specular = factor * frag.material.specular * light.specular;

    // attenuation
    if (light.attenuation && false)
    {
        float distance = length(position - frag.center);
        float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
    }

    return ambient + diffuse + specular;
}

/*
vec3 lighting(Light light, vec3 position, vec3 normal, vec3 eye, float occlusion)
{
    // ambient
    // vec3 ambient = material.ambient * light.ambient * occlusion;
    vec3 ambient = material.ambient * light.ambient;

    if (SSAO)
    {
        ambient *= occlusion;
    }

    float factor;

    // diffuse
    vec3 direction = normalize(light.position - position);
    factor = max(dot(normal, direction), 0.0f);
    vec3 diffuse = factor * material.diffuse * light.diffuse;

    // specular
    vec3 halfway = normalize(direction + normalize(eye - position));
    factor = pow(max(dot(normal, halfway), 0.0f), material.shininess);
    vec3 specular = factor * material.specular * light.specular;

    // distance attenuation
    float distance = length(light.position - position);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}
*/
