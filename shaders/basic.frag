#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in float WorldPosY;
in vec3 Normal_world;
in vec3 FragPos_world;

// Texture samplers
uniform sampler2D textureGrass;
uniform sampler2D textureRock;
uniform sampler2D textureSnow;

// Height thresholds
uniform float heightRockStart;
uniform float heightRockFull;
uniform float heightSnowStart;
uniform float heightSnowFull;
uniform float textureTilingFactor;

// Lighting uniforms
uniform vec3 lightDir_world; // Direction OF the light rays (e.g., from sun)
uniform vec3 lightColor;
uniform float ambientStrength;
uniform vec3 viewPos_world;   // Camera's position in world space
uniform float specularStrength;
uniform int shininess;        // Shininess factor (e.g., 32, 64, 128)

// Fog uniforms
uniform vec3 fogColor;
uniform float fogDensity; // For exponential fog
// OR, for linear fog:
// uniform float fogStart;
// uniform float fogEnd;

void main() {
    vec2 tiledTexCoords = TexCoords * textureTilingFactor;
    vec4 grassColorSample = texture(textureGrass, tiledTexCoords);
    vec4 rockColorSample  = texture(textureRock,  tiledTexCoords);
    vec4 snowColorSample  = texture(textureSnow,  tiledTexCoords);

    float rockBlend = smoothstep(heightRockStart, heightRockFull, WorldPosY);
    float snowBlend = smoothstep(heightSnowStart, heightSnowFull, WorldPosY);

    vec4 materialColor = mix(grassColorSample, rockColorSample, rockBlend);
    materialColor = mix(materialColor, snowColorSample, snowBlend);

    // Lighting calculations
    // Ambient
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal_world);
    vec3 lightDirection = normalize(-lightDir_world); // Vector from fragment to light source
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    vec3 viewDir = normalize(viewPos_world - FragPos_world); // Vector from fragment to camera
    vec3 reflectDir = reflect(-lightDirection, norm); // Reflection of lightDir around normal
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), float(shininess));
    vec3 specular = specularStrength * spec * lightColor; // Light's color affects specular highlight

    vec3 lighting = ambient + diffuse + specular;
    vec3 litColor = lighting * materialColor.rgb;

    // Fog Calculation
    // Option 1: Exponential Fog
    float distToCamera = length(viewPos_world - FragPos_world);
    float fogFactor = exp(-pow(distToCamera * fogDensity, 2.0)); // Exponential squared
    // float fogFactor = exp(-distToCamera * fogDensity); // Simple exponential
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Option 2: Linear Fog (uncomment to use, and comment out Option 1)
    // float distToCamera = length(viewPos_world - FragPos_world);
    // float fogFactor = (fogEnd - distToCamera) / (fogEnd - fogStart);
    // fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Blend final color with fog color
    FragColor = mix(vec4(fogColor, 1.0), vec4(litColor, materialColor.a), fogFactor);
}