#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_color;
uniform sampler2D u_normal;
uniform sampler2D u_depth;

// Thresholds
uniform float u_normalThreshold = 0.4;
uniform float u_depthThreshold = 0.02;
uniform vec4 u_outlineColor;

void main()
{
    vec2 texSize = textureSize(u_color, 0);
    vec2 pixelSize = 1.0 / texSize;

    // 1. Sample Center
    vec3 centerNormal = texture(u_normal, TexCoords).rgb;
    float centerDepth = texture(u_depth, TexCoords).r;

    // 2. Sample Neighbors (Up, Down, Left, Right)
    vec2 offsets[4];
    offsets[0] = vec2(0, 1) * pixelSize;  // Up
    offsets[1] = vec2(0, -1) * pixelSize; // Down
    offsets[2] = vec2(-1, 0) * pixelSize; // Left
    offsets[3] = vec2(1, 0) * pixelSize;  // Right

    bool isEdge = false;

    for(int i = 0; i < 4; i++)
    {
        vec3 neighborNormal = texture(u_normal, TexCoords + offsets[i]).rgb;
        float neighborDepth = texture(u_depth, TexCoords + offsets[i]).r;

        // Normal Difference
        vec3 diffN = abs(centerNormal - neighborNormal);
        if (length(diffN) > u_normalThreshold)
        {
            isEdge = true;
            break;
        }

        // Depth Difference
        float diffD = abs(centerDepth - neighborDepth);
        if (diffD > u_depthThreshold)
        {
            isEdge = true;
            break;
        }
    }

    if (isEdge)
    {
        FragColor = u_outlineColor;
    }
    else
    {
        FragColor = texture(u_color, TexCoords); // Original Game Color
    }
}
