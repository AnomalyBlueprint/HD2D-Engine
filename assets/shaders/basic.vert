#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in float aTexID;

out vec2 TexCoord;
out vec4 VertexColor;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model; // Added for Model Space -> World Space

void main()
{
    // Projection * View * Model * Position
    // Note: Model transform is now done on CPU for Batching!
    // So we just pass aPos directly as World Position.
    // Wait, if CPU does it, aPos is already World Pos?
    // Yes, DrawSprite calculates World Pos.
    // So we don't need Model matrix here anymore for batched sprites.
    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    VertexColor = aColor;
}