#version 450 core

in FragData
{
    flat vec3 albedo;
    flat vec2 number;
} frag;

uniform int OutlineMode;
uniform sampler1D outline;

out vec4 PixelColor;

void main()
{
    // PixelColor = vec4(vec3(0.0f), 1.0f);
    // PixelColor = vec4(frag.albedo, 1.0f);

    float number;

    switch (OutlineMode)
    {
    // residue number
    case 0:
    case 1:
    {
        number = frag.number[1];
        break;
    }
    // atom number
    case 2:
    case 3:
    {
        number = frag.number[0];
        break;
    }
    }

    /*
    switch (OutlineMode)
    {
    case 0:
    {
        PixelColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
        return;
    }
    case 1:
    {
        PixelColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        return;
    }
    case 2:
    {
        PixelColor = vec4(1.0f, 0.0f, 1.0f, 1.0f);
        return;
    }
    case 3:
    {
        PixelColor = vec4(0.0f, 1.0f, 1.0f, 1.0f);
        return;
    }
    }
    */

    float size = 1.0f / textureSize(outline, 0);
    // float offset = frag.number[mode ? 0 : 1] * size + size * 0.5f;
    float offset = number * size + size * 0.5f;

    vec3 color = texture(outline, offset).rgb;

    PixelColor = vec4(color, 1.0f);
}
