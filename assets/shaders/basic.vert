#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in float aTexID;
layout (location = 4) in vec3 aNormal;

out vec2 TexCoord;
out vec4 VertexColor;
out vec3 Normal; // Pass to Frag
out vec3 FragPos; // World Space Position

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model; 

void main()
{
    // Output
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);
    gl_Position = projection * view * worldPos;

    TexCoord = aTexCoord;
    VertexColor = aColor;
    
    // Calculate Normal Matrix
    Normal = mat3(transpose(inverse(model))) * aNormal;
}