#version 450

in FragData
{
    smooth vec2 coords;
} frag;

uniform sampler2D silhouette;
uniform sampler1D outline;
uniform int thickness;

out vec4 PixelColor;

void main()
{
    // 1. campioniamo texel da silhouette
    //    se texel.rbg == white => discard
    //    altrimenti ricaviamo ID atom/residue

    // 2. campioniamo outline con ID trovato
    //    abbiamo il colore dell'eventuale outline

    // 3. controliamo texel vicini per capire se siamo
    //    sul bordo o se abbiamo raggiunto un altro atom/residuo
    //    basta testare texel.rgb != neighbor.rgb
    //    se si' coloriamo con outline color
    //    altrimenti discard

    int w = thickness;
    vec3 texel = texture(silhouette, frag.coords).rgb;

    // retrieve atom/residue number
    uvec3 shift = uvec3(16, 8, 0);
    uvec3 rgb = uvec3(texel * 255.0f);

    uint number = 0;
    for (int j = 0; j < 3; j++)
    {
        number += rgb[j] << shift[j];
    }

    // retrieve outline color
    float size = 1.0f / textureSize(outline, 0);
    float offset = number * size + size * 0.5f;
    vec3 color = texture(outline, offset).rgb;

    // filter value
    if (color == vec3(0.0f))
    {
        discard;
    }

    // if the texel is NOT white
    // (we are on the silhouette)
    if (texel != vec3(1.0f))
    {
        vec2 size = 1.0f / textureSize(silhouette, 0);

        for (int i = -w; i <= +w; i++)
        {
            for (int j = -w; j <= +w; j++)
            {
                if (i == 0 && j == 0)
                {
                    continue;
                }

                vec2 offset = vec2(i, j) * size;

                // and if one of the texel-neighbor has a different color
                // (we are on the border or on another atom/residue)
                if (texture(silhouette, frag.coords + offset).rgb != texel)
                {
                    PixelColor = vec4(color, 1.0f);
                    return;
                }
            }
        }
    }

    discard;
}
