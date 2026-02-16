#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in float aTexID;
layout (location = 4) in vec3 aNormal;

out vec2 TexCoord;
out vec4 VertexColor;
out vec3 Normal; // Pass to Frag

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model; 

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    VertexColor = aColor;
    
    // Calculate Normal Matrix to handle non-uniform scaling
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // Since World Logic handles transformation or Model is Identity for chunks,
    // we can often pass aNormal directly. 
    // BUT for Player, Model matrix has rotation.
    // So we need to transform normal by Model matrix IDT (Inverse Transpose) if scaling,
    // or just Model (rotation) if uniform scaling.
    // For now, let's use the Upper 3x3 of Model matrix to rotate the normal.
    // Output
 
}