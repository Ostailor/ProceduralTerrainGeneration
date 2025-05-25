#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view; // View matrix without translation component

void main() {
    TexCoords = aPos;
    // Remove translation from the view matrix so the skybox follows the camera
    vec4 pos = projection * mat4(mat3(view)) * vec4(aPos, 1.0);
    // Ensure skybox is always drawn at the far clip plane by setting z = w
    // This makes it appear behind everything else.
    gl_Position = pos.xyww; 
}