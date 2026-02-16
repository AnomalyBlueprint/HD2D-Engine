#version 330 core
out vec4 FragColor;

in vec2 TexCoord; 
in vec4 VertexColor;

in vec3 Normal; // Received from Vertex Shader

uniform sampler2D ourTexture;
uniform vec3 u_lightDir;
uniform int u_lightBands;
uniform float u_outlineThickness;
uniform float u_ambientStrength;
uniform bool u_celEnabled;
uniform bool u_outlinesEnabled;

void main()
{
    vec4 texColor = texture(ourTexture, TexCoord) * VertexColor;
    
    // 1. Outlines (UV Space)
    if (u_outlinesEnabled) {
        if (TexCoord.x < u_outlineThickness || TexCoord.x > 1.0 - u_outlineThickness ||
            TexCoord.y < u_outlineThickness || TexCoord.y > 1.0 - u_outlineThickness) {
            // Apply outline (Darken or Black)
            // texColor = vec4(0.0, 0.0, 0.0, 1.0); // Pitch Black Outline
            texColor.rgb *= 0.2; // Darken existing color
        }
    }

    // 2. Cel Shading
    if (u_celEnabled) {
        // Simple directional light
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(-u_lightDir); // Direction *FROM* light source? Usually u_lightDir is direction *TO* light. 
        // Let's assume u_lightDir is direction *TO* light source.
        // Prompt said: "Sun Direction" uniform (e.g., glm::vec3(0.5f, 1.0f, 0.3f))
        
        float diff = max(dot(norm, u_lightDir), 0.0);
        
        // Stepped Lighting
        float level = floor(diff * float(u_lightBands));
        float intensity = level / float(u_lightBands);
        
        // Smoothing constant to avoid pitch black shadows?
        // Add ambient
        vec3 finalColor = (u_ambientStrength + intensity) * texColor.rgb;
        FragColor = vec4(finalColor, texColor.a);
    } else {
        FragColor = texColor;
    }
}