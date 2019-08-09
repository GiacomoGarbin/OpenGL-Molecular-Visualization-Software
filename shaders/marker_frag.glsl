#version 450 core

in vec3 FragColor;

out vec4 PixelColor;

void main()
{
    PixelColor = vec4(FragColor, 1.0f);
}
