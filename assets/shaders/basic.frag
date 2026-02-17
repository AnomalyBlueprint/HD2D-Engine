#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 FragNormal;

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
uniform vec3 u_viewPos;
uniform float u_rimStrength;
uniform int u_debugMode;

in vec3 FragPos;

void main()
{
    vec4 texColor = texture(ourTexture, TexCoord) * VertexColor;
    
    // NOTE: Old UV-based outlines removed. We now assume Post-Process handles it.
    
    // 2. Cel Shading
    if (u_celEnabled) {
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(u_viewPos - FragPos);

        // Diffuse
        float diff = max(dot(norm, u_lightDir), 0.0);
        
        // Stepped Lighting
        float level = floor(diff * float(u_lightBands));
        float intensity = level / float(u_lightBands);
        
        // Rim Lighting (Fresnel)
        float rim = 1.0 - max(dot(viewDir, norm), 0.0);
        rim = smoothstep(0.6, 1.0, rim); // Sharpen rim
        float rimIntensity = rim * u_rimStrength;

        // Ambient Boost: Ensure shadows aren't pitch black (min 0.5)
        float totalLight = max(u_ambientStrength + intensity, 0.5);

        vec3 finalColor = (totalLight * texColor.rgb) + (vec3(rimIntensity) * texColor.rgb);
        FragColor = vec4(finalColor, texColor.a);
    } else {
        FragColor = texColor;
    }

    // Debug Modes
    if (u_debugMode == 1) FragColor = texture(ourTexture, TexCoord); // Texture Only
    if (u_debugMode == 2) FragColor = VertexColor; // Vertex Color Only
    if (u_debugMode == 3) FragColor = vec4(normalize(Normal) * 0.5 + 0.5, 1.0); // Normals

    // Output Raw Normal for Edge Detection
    FragNormal = normalize(Normal);
}