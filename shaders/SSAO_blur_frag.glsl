#version 450 core

// smooth in vec2 offset;
smooth in vec2 coords;

uniform sampler2D occlusion;
uniform int NoiseSize;

out float blur;

void main()
{
    int HalfNoiseSize = int(NoiseSize * 0.5f);
    vec2 size = 1.0f / vec2(textureSize(occlusion, 0));

    blur = 0.0f;

    for (int x = -HalfNoiseSize; x < +HalfNoiseSize; x++)
    {
        for (int y = -HalfNoiseSize; y < +HalfNoiseSize; y++)
        {
            vec2 offset = vec2(x, y) * size;
            blur += texture(occlusion, coords + offset).r;
        }
    }

    blur *= 1.0f / (NoiseSize * NoiseSize);
}
