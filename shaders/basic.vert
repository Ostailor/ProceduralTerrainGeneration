#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out float WorldPosY;
out vec3 Normal_world;
out vec3 FragPos_world;

void main() {
    vec4 worldPosVec4 = model * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPosVec4;
    
    FragPos_world = vec3(worldPosVec4);
    TexCoords = aTexCoords;
    WorldPosY = FragPos_world.y;
    
    Normal_world = mat3(transpose(inverse(model))) * aNormal;
}