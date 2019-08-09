#version 450 core

struct FragData
{
    vec3 center;
    vec3 normal;
    vec3 noise;
};

smooth in vec2 offset;

uniform mat4 projection;

uniform sampler2D center;
uniform sampler2D normal;
uniform sampler2D kernel;
uniform sampler2D noise;

uniform int size; // kernel size
uniform vec2 scale; // noise scale
uniform float radius;
uniform float bias;

out float occlusion;

void main()
{
    FragData frag;
    frag.center = texture(center, offset).xyz;
    frag.normal = normalize(texture(normal, offset).xyz);
    frag.noise = normalize(texture(noise, offset * scale).xyz);

    // TBN matrix
    vec3 tangent = normalize(frag.noise - frag.normal * dot(frag.noise, frag.normal));
    vec3 bitangent = cross(frag.normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, frag.normal);

    occlusion = 0.0f;

    for(int i = 0; i < size; i++)
    {
        float x = float(i) / float(size);

        for(int j = 0; j < size; j++)
        {
            float y = float(j) / float(size);

            vec3 KernelSample = TBN * texture(kernel, vec2(x, y)).xyz;
            KernelSample = frag.center + KernelSample * radius;

            vec4 offset = vec4(KernelSample, 1.0f);
            offset = projection * offset;
            // forse qui dovrei scartare i campioni che vengono clippati
            // perche' vado a campionare qualcosa fuori dal frustum...
            offset.xyz /= offset.w;
            offset.xyz = offset.xyz * 0.5f + 0.5f;

            float depth = texture(center, offset.xy).z;

            // float range = smoothstep(0.0f, 1.0f, radius / abs(frag.center.z - depth));
            // occlusion += ((depth >= KernelSample.z + bias) ? 1.0f : 0.0f) * range;

            // ATTENZIONE
            // depth, frag.center.z e KernelSample.z sono in depth value in view space
            // quindi in generale possono assumere valori fuori dall'intervallo [0, 1]

            // depth rappresenta (in view space) la profondita' della scena
            // del punto campionato cioe' la profondita' del frammento visibile
            // KernelSample.z e' la profondita' del campione che vogliamo testare
            // in pratica rappresenta la profondita' che avrebbe il frammento se
            // scrivessimo nel punto campionato
            // se depth risulta maggiore o uguale di KernelSample.z, significa
            // un frammento della geometria e' stato scritto in quel punto e che
            // si trova piu' vicino alla camera rispetto alla posizione del campione
            // questo perche' siamo in VIEW-SPACE e l'asse positivo delle z e'
            // uscente dallo schermo, perciò un valore (con segno) piu' alto di z
            // in view-space significa un frammento piu' vicino alla camera e viversa


            if (bias < 0.025f)
            {
                // if (abs(frag.center.z - depth) < radius)
                {
                    occlusion += (depth >= KernelSample.z) ? 1.0f : 0.0f;
                }
            }
            else
            {
                float range = smoothstep(0.0f, 1.0f, radius / abs(frag.center.z - depth));
                occlusion += ((depth >= KernelSample.z + bias) ? 1.0f : 0.0f) * range;
            }
        }
    }

    occlusion = 1.0f - (occlusion / float(size * size));
}
