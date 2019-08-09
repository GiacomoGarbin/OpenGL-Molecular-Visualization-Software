#version 450

in FragData
{
    smooth vec2 offset;
} frag;

uniform sampler2D sampler;
uniform int index;

out vec4 PixelColor;

void main()
{
    vec3 color;

    switch (index)
    {
    case -1:
    {
        color = vec3(frag.offset, 0.0f);
        break;
    }
    case +4:
    case +5:
    {
        color = vec3(texture(sampler, frag.offset).r);
        break;
    }
    default:
    {
        color = texture(sampler, frag.offset).rgb;
        break;
    }
    }

    PixelColor = vec4(color, 1.0f);
}
