#version 330 core
out vec4 FragColor;

in vec2 TexCoord; // <--- Input from Vertex Shader

uniform sampler2D ourTexture; // <--- The Texture Data

void main()
{
    FragColor = texture(ourTexture, TexCoord);
}